/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    $RCSfile: itkTracePathOnImageFilter.txx,v $
Language:  C++
Date:      $Date: 2005/07/27 15:21:11 $
Version:   $Revision: 1.7 $

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkTracePathOnImageFilter_txx
#define __itkTracePathOnImageFilter_txx

#include "steps/inc/itk_trace_path_on_image_filter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkPathIterator.h"
#include "itkNumericTraits.h"

namespace itk
{

/** Constructor */
template <class TInputPath, class TOutputImage>
TracePathOnImageFilter<TInputPath,TOutputImage>
::TracePathOnImageFilter()
{
  //this->SetNumberOfRequiredInputs(1);
  m_PathValue = NumericTraits<ValueType>::One;
  m_BackgroundValue = NumericTraits<ValueType>::Zero;
}

template <class TInputPath, class TOutputImage>
void
TracePathOnImageFilter<TInputPath,TOutputImage>
::setNumberOfPaths(int NoP)
{
	this->SetNumberOfRequiredInputs(NoP);
}

template <class TInputPath, class TOutputImage>
void
TracePathOnImageFilter<TInputPath,TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Path Value : " << m_PathValue << std::endl;
  os << indent << "Background Value : " << m_BackgroundValue << std::endl;
}

//----------------------------------------------------------------------------
template <class TInputPath, class TOutputImage>
void
TracePathOnImageFilter<TInputPath,TOutputImage>
::GenerateData(void)
{
  //itkDebugMacro(<< "TracePathOnImageFilter::GenerateData() called");
  this->CalculatePathRegion();
  this->AllocateOutputs();   // allocate the image                            
  // Get the input and output pointers 
  //const InputPathType * InputPath   = this->GetInput();
  OutputImagePointer    OutputImage = this->GetOutput();
  
  ImageRegionIteratorWithIndex<OutputImageType> imageIt(OutputImage,
    OutputImage->GetRequestedRegion());
  for( imageIt.GoToBegin(); !imageIt.IsAtEnd(); ++imageIt )
    {
    imageIt.Set(m_BackgroundValue);
    }
  
  //multiple paths
	for (int i = 0; i < this->ProcessObject::GetNumberOfInputs(); ++i)
	{
		const InputPathType * InputPath   = this->GetInput(i);

		PathIterator<OutputImageType,InputPathType> pathIt(OutputImage,InputPath);
		for( pathIt.GoToBegin(); !pathIt.IsAtEnd(); ++pathIt )
		{
			pathIt.Set( m_PathValue );
		}
	}
  
  //itkDebugMacro(<< "TracePathOnImageFilter::GenerateData() finished");
} // end update function 


} // end namespace itk

#endif
