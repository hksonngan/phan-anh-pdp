namespace itk
{
	
template <class TInputImage, class TFeatureImage, class TOutputImage, class TFunction, class TSharedData, typename TIdCell>
void StartStopChanVese<TInputImage, TFeatureImage, TOutputImage, TFunction, TSharedData, TIdCell>
::Start()
{
	std::cout << "StartStopChanVese::Start()" << std::endl;

	//if( ! this->m_InitializedState )
    //{
    // Set the coefficients for the deriviatives
    double coeffs[ImageDimension];
    unsigned int i;
    if (m_UseImageSpacing)
      {
      for (i = 0; i < ImageDimension; i++)
        {
        coeffs[i] = 1.0 / m_LevelSet[0]->GetSpacing()[i];
        }
      }
    else
      {
      for (i = 0; i < ImageDimension; i++)
        {
        coeffs[i] = 1.0;
        }
      }

    for( IdCellType id = 0; id <  this->m_FunctionCount; id++ )
    {
      this->m_DifferenceFunctions[id]->SetScaleCoefficients(coeffs);
    }

    // Allocate the output image -- inherited method
    //this->AllocateOutputs()
	//// Allocate the output image
	typename TOutputImage::Pointer output = this->GetOutput();
	output->SetBufferedRegion(output->GetRequestedRegion());
	output->Allocate();

	//// Copy the input image to the output image.  Algorithms will operate
	//// directly on the output image and the update buffer.
	this->CopyInputToOutput();

	//// Perform any other necessary pre-iteration initialization.
	this->Initialize();

	//// Allocate the internal update buffer.  This takes place entirely within
	//// the subclass, since this class cannot define an update buffer type.
	this->AllocateUpdateBuffer();

	//// Reset the number of elapsed iterations
	this->SetElapsedIterations(0);

	//// We have been initialized
	m_Started = true; 

}

template <class TInputImage, class TFeatureImage, class TOutputImage, class TFunction, class TSharedData, typename TIdCell>
void StartStopChanVese<TInputImage, TFeatureImage, TOutputImage, TFunction, TSharedData, TIdCell>
::Run(unsigned int nIterations) 
{
	std::cout << "StartStopChanVese::Run()" << std::endl;

	// Better be initialized
	assert(m_Started);

	for(unsigned int i=0;i<nIterations;i++)
	{
		this->InitializeIteration();
		this->m_RMSChange = NumericTraits<double>::max();

		TimeStepType dt = this->CalculateChange();

		this->ApplyUpdate(dt);

		this->SetElapsedIterations( this->GetElapsedIterations() + 1);

		
		 // Invoke the iteration event.
		this->InvokeEvent( IterationEvent() );

		if( this->GetAbortGenerateData() )
		  {
		  this->InvokeEvent( IterationEvent() );
		  this->ResetPipeline();
		  throw ProcessAborted(__FILE__,__LINE__);
		  }

		this->InitializeIteration();

	}    

	// Any further processing of the solution can be done here.  
	this->PostProcessOutput();

	std::cout << "Number of layers: " << this->GetNumberOfLayers() << "\n"; 
}

template <class TInputImage, class TFeatureImage, class TOutputImage, class TFunction, class TSharedData, typename TIdCell>
void StartStopChanVese<TInputImage, TFeatureImage, TOutputImage, TFunction, TSharedData, TIdCell>
::GenerateData()
{
	std::cout << "StartStopChanVese::GenerateData()" << std::endl;
	if(!m_Started)
		Start();
}

template <class TInputImage, class TFeatureImage, class TOutputImage, class TFunction, class TSharedData, typename TIdCell>
StartStopChanVese<TInputImage, TFeatureImage, TOutputImage, TFunction, TSharedData, TIdCell>
::StartStopChanVese()
{
	std::cout << "StartStopChanVese::StartStopChanVese()" << std::endl;
	m_Started = false;
}

template <class TInputImage, class TFeatureImage, class TOutputImage, class TFunction, class TSharedData, typename TIdCell>
void StartStopChanVese<TInputImage, TFeatureImage, TOutputImage, TFunction, TSharedData, TIdCell>
::PrintSelf(std::ostream &os, itk::Indent indent) const
{
	std::cout << "StartStopChanVese::PrintSelf" << std::endl;
	Superclass::PrintSelf(os, indent);

	os << indent << "Running: " << m_Started << std::endl;
}

template <class TInputImage, class TFeatureImage, class TOutputImage, class TFunction, class TSharedData, typename TIdCell>
void StartStopChanVese<TInputImage, TFeatureImage, TOutputImage, TFunction, TSharedData, TIdCell>
:: CallUpdate()
{
	std::cout << "StartStopChanVese::CallUpdate()" << std::endl;
	Superclass::Update();
	this->Update();
}

}  // namespace