/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkPathToImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2005/07/27 15:21:11 $
  Version:   $Revision: 1.3 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkPathToImageFilter_h
#define __itkPathToImageFilter_h

#include <itkImageSource.h>
#include <itkConceptChecking.h>
//#include "itkMacro.h"

namespace itk
{
  
/** \class PathToImageFilter
 * \brief Base class for filters that take a Path as input and produce an image as output.
 * Base class for filters that take a Path as input and produce an image as
 * output. By default, if the user does not specify the region of the output
 * image, the path's bounding box is used. 
 * Note: this computation may be relatively slow for some path types. If possible,
 * manually set the output image region via GetOutput()->SetRequestedRegion() and
 * call CalculatePathRegionOff() to tell the filter not to find the input path's
 * region.
 * To get or set any of the properties of the output image (e.g. origin, spacing, or
 * region information as above), do so directly: e.g. GetOutput()->SetSpacing(...).
 * 
 * \warning The path region computations are made by the CalculatePathRegion()
 * method. This method MUST be called in GenerateData(), before AllocateOutputs()
 * has been called. (Calling it any other time won't work, because the input 
 * path has to be fully-defined, which can only be guaranteed at the time of
 * GenerateData().) Thus, any subclasses that wish to calculate the path region
 * and use that as the output region MUST call CalculatePathRegion() before they
 * call AllocateOutputs() in GenerateData();
 * 
 * \warning This class used to contain functionality to trace a path onto an image. 
 * Such functionality is inconsistent with the role of this class as a virtual
 * base class. This functionality has been deprecated from this class and moved
 * into TracePathOnImageFilter.
 */
template <class TInputPath, class TOutputImage>
class ITK_EXPORT PathToImageFilter : public ImageSource<TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef PathToImageFilter  Self;
  typedef ImageSource<TOutputImage>  Superclass;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(PathToImageFilter,ImageSource);
  
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

  /** Set/Get the path input of this process object.  */
  virtual void SetInput( const InputPathType *path);
  virtual void SetInput( unsigned int, const TInputPath * path);
  const InputPathType * GetInput(void);
  const InputPathType * GetInput(unsigned int idx);
  
  /** Control whether the filter should attempt to calculate the path's bounding
   * box, or the RequestedRegion set on the output image should be used. */
  itkSetMacro(CalculatePathRegion, bool);
  itkGetMacro(CalculatePathRegion, bool);
  itkBooleanMacro(CalculatePathRegion);
  
  /** \deprecated As of ITK 2.6, set/get the spacing from the output directly via GetOutput()->GetSpacing() */
  itkLegacyMacro(virtual void SetSpacing( const double* spacing));
  itkLegacyMacro(virtual void SetSpacing( const float* spacing));
  itkLegacyMacro(virtual const double* GetSpacing() const);

  /** \deprecated This filter is a base class. To draw a path on an image,
  * use TracePathOnImageFilter.
  * Set/Get the value for pixels on and off the path. 
  * By default, this filter will return a "0" image with path pixels set to 1 */
  itkSetMacro(PathValue, ValueType);
  itkGetMacro(PathValue, ValueType);
  itkSetMacro(BackgroundValue, ValueType);
  itkGetMacro(BackgroundValue, ValueType);

  /** \deprecated As of ITK 2.6, set/get the origin from the output directly via GetOutput()->GetOrigin() */
  itkLegacyMacro(virtual void SetOrigin( const double* origin));
  itkLegacyMacro(virtual void SetOrigin( const float* origin));
  itkLegacyMacro(virtual const double * GetOrigin() const);

  /** \deprecated As of ITK 2.6, set/get the size from the output directly via GetOutput()->GetRequestedRegion() */
  itkLegacyMacro(virtual void SetSize(const SizeType s));
  itkLegacyMacro(virtual SizeType GetSize());

  
protected:
  PathToImageFilter();
  ~PathToImageFilter() {};
  
  // do nothing: can't copy information from a path to an image!
  virtual void GenerateOutputInformation() {}
  virtual void CalculatePathRegion();

  virtual void PrintSelf(std::ostream& os, Indent indent) const;
  virtual void GenerateData();

  // SLATED FOR REMOVAL AFTER ITK 2.8:
  ValueType    m_PathValue;
  ValueType    m_BackgroundValue;


private:
  PathToImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  bool         m_CalculatePathRegion;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "steps/inc/itk_path_to_image_filter.txx"
#endif

#endif
