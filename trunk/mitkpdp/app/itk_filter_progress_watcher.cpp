#include "itk_filter_progress_watcher.h"
#include "app/thickening_detection_step_base.h"

pdp::itkFilterProgressWatcher::itkFilterProgressWatcher(itk::ProcessObject* obj, ThickeningDetectionStep* step, float from, float to, int stepPerIteration, int totalIterations)
    : m_step(step)
    , m_from(from)
    , m_to(to)
    , m_totalSteps(stepPerIteration)
    , m_totalIterations(totalIterations)
    , m_steps(0)
    , m_iterations(0)
    , m_process(obj)
{
    // Create a series of commands
    m_startFilterCommand = CommandType::New();
    m_endFilterCommand = CommandType::New();
    m_progressFilterCommand = CommandType::New();
    m_iterationFilterCommand = CommandType::New();

    // Assign the callbacks
    m_startFilterCommand->SetCallbackFunction(this, &itkFilterProgressWatcher::startFilter);
    m_endFilterCommand->SetCallbackFunction(this, &itkFilterProgressWatcher::endFilter);
    m_progressFilterCommand->SetCallbackFunction(this, &itkFilterProgressWatcher::showProgress);
    m_iterationFilterCommand->SetCallbackFunction(this, &itkFilterProgressWatcher::showIteration);

    // Add the commands as observers
    m_startTag = m_process->AddObserver(itk::StartEvent(), m_startFilterCommand);
    m_endTag = m_process->AddObserver(itk::EndEvent(), m_endFilterCommand);
    m_progressTag = m_process->AddObserver(itk::ProgressEvent(), m_progressFilterCommand);
    m_iterationTag = m_process->AddObserver(itk::IterationEvent(), m_iterationFilterCommand);
}

pdp::itkFilterProgressWatcher::~itkFilterProgressWatcher()
{
    // Remove any observers we have on the old process object
    if(m_process) {
        if(m_startFilterCommand) {
            m_process->RemoveObserver(m_startTag);
        }
        if(m_endFilterCommand) {
            m_process->RemoveObserver(m_endTag);
        }
        if(m_progressFilterCommand) {
            m_process->RemoveObserver(m_progressTag);
        }
        if(m_iterationFilterCommand) {
            m_process->RemoveObserver(m_iterationTag);
        }
    }
}

void pdp::itkFilterProgressWatcher::updateProgressbar()
{
    float doneInPrevIters = float(m_iterations)/float(m_totalIterations);
    float doneInThisIter = float(m_steps)/float(m_totalSteps)/float(m_totalIterations);
    float done = doneInPrevIters + doneInThisIter;
    // Clamp in range [from, to] for the case of wrong totalstep or totaliter count.
    float progress = std::min(std::max(m_from + done*(m_to - m_from), m_from), m_to);
    m_step->emitStepProgress(progress);
}

void pdp::itkFilterProgressWatcher::showProgress()
{
    updateProgressbar();
    m_steps++;
}

void pdp::itkFilterProgressWatcher::showIteration()
{
    updateProgressbar();
    m_iterations++;
}

void pdp::itkFilterProgressWatcher::startFilter()
{
    m_steps = 0;
    m_iterations = 0;
    m_step->emitStepProgress(m_from);
}

void pdp::itkFilterProgressWatcher::endFilter()
{
    m_step->emitStepProgress(m_to);
}
