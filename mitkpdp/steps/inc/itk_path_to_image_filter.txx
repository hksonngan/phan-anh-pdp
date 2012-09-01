/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    $RCSfile: itkPathToImageFilter.txx,v $
Language:  C++
Date:      $Date: 2005/07/27 15:21:11 $
Version:   $Revision: 1.7 $

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#pragma once

#include <itkPathToImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkPathIterator.h>
#include <itkNumericTraits.h>
#include <itkPolyLineParametricPath.h>
#include <itkImageRegion.h>

#include <vcl_cmath.h>

/** Calculate the bounding box from an input path.
  * We provide a generic version and an explicit specialization for various
  * PolyLineParametricPath types.
*/
template<class TPath>
itk::ImageRegion<TPath::PathDimension>
InternalCalculatePathRegion(const TPath* paths[], unsigned int numPaths);

namespace itk
{

/** Constructor */
template <class TInputPath, class TOutputImage>
PathToImageFilter<TInputPath,TOutputImage>
::PathToImageFilter()
{
  m_CalculatePathRegion = true;

  // SLATED FOR REMOVAL AFTER ITK 2.8
  m_PathValue = NumericTraits<ValueType>::One;
  m_BackgroundValue = NumericTraits<ValueType>::Zero;
}
  

/** Set the Input SpatialObject */
template <class TInputPath, class TOutputImage>
void
PathToImageFilter<TInputPath,TOutputImage>
::SetInput(const InputPathType *input)
{
  this->SetInput(0, input);
}


/** Connect one of the operands  */
template <class TInputPath, class TOutputImage>
void
PathToImageFilter<TInputPath,TOutputImage>
::SetInput( unsigned int index, const InputPathType *input) 
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(index, const_cast< InputPathType * >( input ) );
}

/** Get the input Path */
template <class TInputPath, class TOutputImage>
const typename PathToImageFilter<TInputPath,TOutputImage>::InputPathType *
PathToImageFilter<TInputPath,TOutputImage>
::GetInput(void) 
{
  if (this->ProcessObject::GetNumberOfInputs() < 1)
    {
    return 0;
    }
  
  return static_cast<const TInputPath * >
    (this->ProcessObject::GetInput(0) );
}
  
/** Get the input Path */
template <class TInputPath, class TOutputImage>
const typename PathToImageFilter<TInputPath,TOutputImage>::InputPathType *
PathToImageFilter<TInputPath,TOutputImage>
::GetInput(unsigned int idx)
{
  return static_cast< const TInputPath * >
    (this->ProcessObject::GetInput(idx));
}

template <class TInputPath, class TOutputImage>
void
PathToImageFilter<TInputPath,TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "CalculatePathRegion : " << m_CalculatePathRegion << std::endl;
  // REMOVE BELOW AFTER ITK 2.6
  os << indent << "Path Value : " << m_PathValue << std::endl;
  os << indent << "Background Value : " << m_BackgroundValue << std::endl;
}

template <class TInputPath, class TOutputImage>
void
PathToImageFilter<TInputPath,TOutputImage>
::CalculatePathRegion()
{
  if (m_CalculatePathRegion)
    {
    unsigned int numInputs = this->GetNumberOfInputs();
    const InputPathType** inputs = new const InputPathType*[numInputs];
    for (unsigned int i = 0; i < numInputs; i++) inputs[i] = this->GetInput(i);
    // Is 'SetRegions()' correct? SetRequestedRegion() alone causes errors
    // because then the requested region is outside the largest possible region.
    this->GetOutput()->SetRegions(InternalCalculatePathRegion<InputPathType>(inputs, numInputs));
    delete[] inputs;
    }
}


} // end namespace itk

template<class TPath>
itk::ImageRegion<TPath::PathDimension>
InternalCalculatePathRegion(const TPath *paths[], unsigned int numPaths)
{
  typedef typename TPath::ConstPointer PathPointer;
  typedef typename TPath::IndexType    IndexType;
  typedef typename TPath::InputType    InputType;
  typedef typename TPath::OffsetType   OffsetType;
  typedef itk::ImageRegion<TPath::PathDimension> RegionType;
  typedef typename RegionType::SizeType SizeType;
  
  PathPointer path = paths[0];
  IndexType low, high;
  low = high = path->EvaluateToIndex(path->StartOfInput());
  OffsetType zeroOffset;
  zeroOffset.Fill(0);
  for(unsigned int i = 0; i < numPaths; i++)
    {
    path = paths[i];
    if (path.IsNull()) break;
    InputType position = path->StartOfInput();
    do
      {
        IndexType current = path->EvaluateToIndex(position);
        for(unsigned int j = 0; j < TPath::PathDimension; j++)
          {
          if (current[j] < low[j]) low[j] = current[j];
          if (current[j] > high[j]) high[j] = current[j];
          }
      }
    while(path->IncrementInput(position) != zeroOffset);    
    }
  
  SizeType size;
  for(unsigned int j = 0; j < TPath::PathDimension; j++)
    {
    size[j] = high[j] - low[j] + 1;
    }
  return RegionType(low, size);
}

template<>
itk::ImageRegion<2>
InternalCalculatePathRegion<itk::PolyLineParametricPath<2> >(const itk::PolyLineParametricPath<2> *paths[], unsigned int numPaths)
{
  typedef itk::PolyLineParametricPath<2> TPath;
  typedef TPath::ConstPointer PathPointer;
  typedef TPath::IndexType    IndexType;
  typedef TPath::InputType    InputType;
  typedef TPath::OffsetType   OffsetType;
  typedef TPath::ContinuousIndexType ContinuousIndexType;
  typedef itk::ImageRegion<TPath::PathDimension> RegionType;
  typedef RegionType::SizeType SizeType;
  
  ContinuousIndexType low, high;
  PathPointer path = paths[0];
  low = high = path->GetVertexList()->ElementAt(0);
  for(unsigned int i = 0; i < numPaths; i++)
    {
    path = paths[i];
    if (path.IsNull()) break;
    TPath::VertexListType::ConstPointer vertices = path->GetVertexList();
    for(TPath::VertexListType::ConstIterator vit = vertices->Begin();
        vit != vertices->End(); ++vit)
      {
        const ContinuousIndexType &current(vit.Value());
        if (current[0] < low[0]) low[0] = current[0];
        if (current[0] > high[0]) high[0] = current[0];
        if (current[1] < low[1]) low[1] = current[1];
        if (current[1] > high[1]) high[1] = current[1];
      }
    } // end for-all-inputs loop
  
  SizeType size;
  IndexType start;
  // round properly so that the entire path is within the image
  start[0] = static_cast<IndexType::IndexValueType>(floor(low[0]));
  size[0] = static_cast<SizeType::SizeValueType>(ceil(high[0] - low[0])) + 1;
  start[1] = static_cast<IndexType::IndexValueType>(floor(low[1]));
  size[1] = static_cast<SizeType::SizeValueType>(ceil(high[1] - low[1])) + 1;
  
  return RegionType(start, size);
}

template<>
itk::ImageRegion<3>
InternalCalculatePathRegion<itk::PolyLineParametricPath<3> >(const itk::PolyLineParametricPath<3> *paths[], unsigned int numPaths)
{
  typedef itk::PolyLineParametricPath<3> TPath;
  typedef TPath::ConstPointer PathPointer;
  typedef TPath::IndexType    IndexType;
  typedef TPath::InputType    InputType;
  typedef TPath::OffsetType   OffsetType;
  typedef TPath::ContinuousIndexType ContinuousIndexType;
  typedef itk::ImageRegion<TPath::PathDimension> RegionType;
  typedef RegionType::SizeType SizeType;
  
  ContinuousIndexType low, high;
  PathPointer path = paths[0];
  low = high = path->GetVertexList()->ElementAt(0);
  for(unsigned int i = 0; i < numPaths; i++)
    {
    path = paths[i];
    if (path.IsNull()) break;
    TPath::VertexListType::ConstPointer vertices = path->GetVertexList();
    for(TPath::VertexListType::ConstIterator vit = vertices->Begin();
        vit != vertices->End(); ++vit)
      {
      const ContinuousIndexType &current(vit.Value());
      if (current[0] < low[0]) low[0] = current[0];
      if (current[0] > high[0]) high[0] = current[0];
      if (current[1] < low[1]) low[1] = current[1];
      if (current[1] > high[1]) high[1] = current[1];
      if (current[2] < low[2]) low[2] = current[2];
      if (current[2] > high[2]) high[2] = current[2];
      }
    } // end for-all-inputs loop
  
  SizeType size;
  IndexType start;
  // round properly so that the entire path is within the image
  start[0] = static_cast<IndexType::IndexValueType>(floor(low[0]));
  size[0] = static_cast<SizeType::SizeValueType>(ceil(high[0] - low[0])) + 1;
  start[1] = static_cast<IndexType::IndexValueType>(floor(low[1]));
  size[1] = static_cast<SizeType::SizeValueType>(ceil(high[1] - low[1])) + 1;
  start[2] = static_cast<IndexType::IndexValueType>(floor(low[2]));
  size[2] = static_cast<SizeType::SizeValueType>(ceil(high[2] - low[2])) + 1;
  
  return RegionType(start, size);
}

// REMOVE ALL DEPRECATED FUNCTIONALITY BELOW AFTER ITK 2.6
//----------------------------------------------------------------------------
 namespace itk
{

template <class TInputPath, class TOutputImage>
void
PathToImageFilter<TInputPath,TOutputImage>
::SetSpacing(const double* spacing)
{
  itkLegacyReplaceBodyMacro(PathtoImageFilter::SetSpacing, 2.6, PathtoImageFilter::GetOutput()->SetSpacing(...));
  this->GetOutput()->SetSpacing(spacing);
}

template <class TInputPath, class TOutputImage>
void
PathToImageFilter<TInputPath,TOutputImage>
::SetSpacing(const float* spacing)
{
  itkLegacyReplaceBodyMacro(PathtoImageFilter::SetSpacing, 2.6, PathtoImageFilter::GetOutput()->SetSpacing(...));
  this->GetOutput()->SetSpacing(spacing);
}

template <class TInputPath, class TOutputImage>
const double * 
PathToImageFilter<TInputPath,TOutputImage>
::GetSpacing() const
{
  itkLegacyReplaceBodyMacro(PathtoImageFilter::GetSpacing, 2.6, PathtoImageFilter::GetOutput()->GetSpacing());
  // const-cast because ImageSource defines no const GetOutput method!
  return const_cast<Self*>(this)->GetOutput()->GetSpacing().GetDataPointer();
}

template <class TInputPath, class TOutputImage>
void
PathToImageFilter<TInputPath,TOutputImage>
::SetOrigin(const double* origin)
{
  itkLegacyReplaceBodyMacro(PathtoImageFilter::SetOrigin, 2.6, PathtoImageFilter::GetOutput()->SetOrigin(...));
  this->GetOutput()->SetOrigin(origin);
}

template <class TInputPath, class TOutputImage>
void
PathToImageFilter<TInputPath,TOutputImage>
::SetOrigin(const float* origin)
{
  itkLegacyReplaceBodyMacro(PathtoImageFilter::SetOrigin, 2.6, PathtoImageFilter::GetOutput()->SetOrigin(...));
  this->GetOutput()->SetOrigin(origin);
}

template <class TInputPath, class TOutputImage>
const double * 
PathToImageFilter<TInputPath,TOutputImage>
::GetOrigin() const
{
  itkLegacyReplaceBodyMacro(PathtoImageFilter::GetOrigin, 2.6, PathtoImageFilter::GetOutput()->GetOrigin());
  // const-cast because ImageSource defines no const GetOutput method!
  return const_cast<Self*>(this)->GetOutput()->GetOrigin().GetDataPointer();
}

template <class TInputPath, class TOutputImage>
void
PathToImageFilter<TInputPath,TOutputImage>
::SetSize(const SizeType s)
{
  itkLegacyReplaceBodyMacro(PathtoImageFilter::SetSize, 2.6, PathtoImageFilter::GetOutput()->SetRequestedRegion(...));
  OutputImageRegionType r = this->GetOutput()->GetRequestedRegion();
  r.SetSize(s);
  this->GetOutput()->SetRequestedRegion(r);
}

template <class TInputPath, class TOutputImage>
typename PathToImageFilter<TInputPath,TOutputImage>
::SizeType 
PathToImageFilter<TInputPath,TOutputImage>
::GetSize()
{
  itkLegacyReplaceBodyMacro(PathtoImageFilter::GetSize, 2.6, PathtoImageFilter::GetOutput()->GetRequestedRegion().GetSize());
  // const-cast because ImageSource defines no const GetOutput method!
  return this->GetOutput()->GetRequestedRegion().GetSize();
}


template <class TInputPath, class TOutputImage>
void
PathToImageFilter<TInputPath,TOutputImage>
::GenerateData(void)
{
  itkWarningMacro("As of ITK 2.6, PathToImageFilter is a base class and should not be used directly. Use TracePathOnImageFilter instead.")
  //itkDebugMacro(<< "TracePathOnImageFilter::GenerateData() called");
  this->CalculatePathRegion();
  this->AllocateOutputs();   // allocate the image                            
  
  // Get the input and output pointers 
  const InputPathType * InputPath   = this->GetInput();
  OutputImagePointer    OutputImage = this->GetOutput();
  
  ImageRegionIteratorWithIndex<OutputImageType> imageIt(OutputImage,
    OutputImage->GetRequestedRegion());
  for( imageIt.GoToBegin(); !imageIt.IsAtEnd(); ++imageIt )
    {
    imageIt.Set(m_BackgroundValue);
    }
  
  PathIterator<OutputImageType,InputPathType> pathIt(OutputImage,InputPath);
  for( pathIt.GoToBegin(); !pathIt.IsAtEnd(); ++pathIt )
    {
    pathIt.Set( m_PathValue );
    }
  
  //itkDebugMacro(<< "TracePathOnImageFilter::GenerateData() finished");
} // end update function 

} // end namespace itk
