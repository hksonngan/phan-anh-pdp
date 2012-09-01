/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkTracePathOnImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2005/07/27 15:21:11 $
  Version:   $Revision: 1.3 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkTracePathOnImageFilter_h
#define __itkTracePathOnImageFilter_h

#include "steps/inc/itk_path_to_image_filter.h"

namespace itk
{
  
/** \class TracePathOnImageFilter
 * \brief Trace a given path on an image.
 * This class traces a path out onto an image. Use SetPathValue and SetBackgroundValue
 * to control the value written to pixels on and off the path (respectively).
 *  
 * If CalculatePathRegionOff() (a method of the superclass) then the image region 
 * must be set manually by filter->GetOutput()->SetRequestedRegion(...). 
 * Otherwise the region is calculated from the path's bounding box.
 * To set other properties of the output image, use the filter->GetOutput()->Set...
 * technique.
  
 */
template <class TInputPath, class TOutputImage>
class ITK_EXPORT TracePathOnImageFilter : public PathToImageFilter<TInputPath, TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef TracePathOnImageFilter  Self;
  typedef PathToImageFilter<TInputPath, TOutputImage>  Superclass;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(TracePathOnImageFilter,PathToImageFilter);
  
  /** Some convenient typedefs. */
  typedef          TInputPath                         InputPathType;
  typedef typename InputPathType::Pointer             InputPathPointer;
  typedef typename InputPathType::ConstPointer        InputPathConstPointer;
  typedef          TOutputImage                       OutputImageType;
  typedef typename OutputImageType::Pointer           OutputImagePointer;
  typedef typename OutputImageType::SizeType          SizeType;
  typedef typename OutputImageType::ValueType         ValueType;
  typedef typename OutputImageType::IndexType         IndexType;
  typedef typename OutputImageType::RegionType        OutputImageRegionType;

  /** ImageDimension constants */
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TOutputImage::ImageDimension);

  /** Set/Get the value for pixels on and off the path. 
  * By default, this filter will return a "0" image with path pixels set to 1 */
  itkSetMacro(PathValue, ValueType);
  itkGetMacro(PathValue, ValueType);
  itkSetMacro(BackgroundValue, ValueType);
  itkGetMacro(BackgroundValue, ValueType);

  virtual void setNumberOfPaths(int NoP);


protected:
  TracePathOnImageFilter();
  ~TracePathOnImageFilter() {};

  virtual void PrintSelf(std::ostream& os, Indent indent) const;

private:
  TracePathOnImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  void GenerateData();
  
  ValueType    m_PathValue;
  ValueType    m_BackgroundValue;

  //int numberOfPaths;
  
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "steps/inc/itk_trace_path_on_image_filter.txx"
#endif

#endif
