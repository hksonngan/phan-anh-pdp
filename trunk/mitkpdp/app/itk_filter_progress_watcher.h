#pragma once

#include <itkProcessObject.h>
#include <itkCommand.h>

namespace pdp {
    class ThickeningDetectionStep;

class itkFilterProgressWatcher {
public:
    itkFilterProgressWatcher(itk::ProcessObject* o, ThickeningDetectionStep* step, float from, float to, int totalsteps_per_iteration = 100, int m_totaliterations = 1);
    virtual ~itkFilterProgressWatcher();

protected:
    virtual void showProgress();
    virtual void showIteration();
    virtual void startFilter();
    virtual void endFilter();
    void updateProgressbar();

private:
    ThickeningDetectionStep* m_step;
    float m_from;
    float m_to;
    int m_totalSteps;
    int m_totalIterations;
    int m_steps;
    int m_iterations;
    std::string m_comment;
    itk::ProcessObject::Pointer m_process;

    typedef itk::SimpleMemberCommand<itkFilterProgressWatcher> CommandType;
    CommandType::Pointer m_startFilterCommand;
    CommandType::Pointer m_endFilterCommand;
    CommandType::Pointer m_progressFilterCommand;
    CommandType::Pointer m_iterationFilterCommand;

    unsigned long m_startTag;
    unsigned long m_endTag;
    unsigned long m_progressTag;
    unsigned long m_iterationTag;
};

}
