/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkOtsuThresholdImageCalculator.h,v $
  Language:  C++
  Date:      $Date: 2009-04-23 03:53:36 $
  Version:   $Revision: 1.9 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#pragma once

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkNumericTraits.h>

namespace itk
{

/** \class OtsuThresholdImageCalculator
 * \brief Computes the Otsu's threshold for an image.
 * 
 * This calculator computes the Otsu's threshold which separates an image
 * into foreground and background components. The method relies on a
 * histogram of image intensities. The basic idea is to maximize the 
 * between-class variance.
 *
 * This class is templated over the input image type.
 *
 * \warning This method assumes that the input image consists of scalar pixel
 * types.
 *
 * \ingroup Operators
 */
template <class TInputImage>
class ITK_EXPORT OtsuThresholdImageCalculator : public Object 
{
public:
  /** Standard class typedefs. */
  typedef OtsuThresholdImageCalculator Self;
  typedef Object                       Superclass;
  typedef SmartPointer<Self>           Pointer;
  typedef SmartPointer<const Self>     ConstPointer;

  typedef typename itk::Image<unsigned char, 3> MaskType;
  typedef typename MaskType::Pointer MaskPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(OtsuThresholdImageCalculator, Object);

  /** Type definition for the input image. */
  typedef TInputImage  ImageType;

  /** Pointer type for the image. */
  typedef typename TInputImage::Pointer  ImagePointer;
  
  /** Const Pointer type for the image. */
  typedef typename TInputImage::ConstPointer ImageConstPointer;

  /** Type definition for the input image pixel type. */
  typedef typename TInputImage::PixelType PixelType;
  
  /** Type definition for the input image region type. */
  typedef typename TInputImage::RegionType RegionType;

  /** Type definition for the input image index type. */
  typedef typename TInputImage::IndexType IndexType;
  
  /** Set the input image. */
  itkSetConstObjectMacro(Image,ImageType);

  /** Compute the Otsu's threshold for the input image. */
  void Compute(void);

  /** Compute the supervised range constrained Otsu's threshold for the input image. */
  void ComputeSVRC(void);

  /** Compute the supervised range constrained Otsu's threshold for the input image using mask*/
  void ComputeSVRCWithMask(void);

  /** Compute the min max values for the input image using mask*/
  void MinMaxMask(void);

  /** Return the Otsu's threshold value. */
  itkGetConstMacro(Threshold,PixelType);
  
  /** Set/Get the number of histogram bins. Default is 128. */
  itkSetClampMacro( NumberOfHistogramBins, unsigned long, 1, 
                    NumericTraits<unsigned long>::max() );
  itkGetConstMacro( NumberOfHistogramBins, unsigned long );

  /** Set the region over which the values will be computed */
  void SetRegion( const RegionType & region );

  void SetLowerBound(float lb)
  {
	  m_LowerBound = lb;
  }

  void SetUpperBound(float ub)
  {
	  m_UpperBound = ub;
  }

  void SetMask(MaskPointer mask)
  {
	  m_Mask = mask;
  }

  void SetMaskOnOff(bool mask)
  {
	  isMask = mask;
  }

protected:
  OtsuThresholdImageCalculator();
  virtual ~OtsuThresholdImageCalculator() {};
  void PrintSelf(std::ostream& os, Indent indent) const;

private:
  OtsuThresholdImageCalculator(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  
  PixelType            m_Threshold;
  unsigned long        m_NumberOfHistogramBins;
  ImageConstPointer    m_Image;
  RegionType           m_Region;
  bool                 m_RegionSetByUser;
  float				   m_LowerBound;
  float				   m_UpperBound;
  MaskPointer		   m_Mask;
  bool				   isMask;

  PixelType            m_Minimum;
  PixelType            m_Maximum;

  IndexType            m_IndexOfMinimum;
  IndexType            m_IndexOfMaximum;

};

} // end namespace itk


#ifndef ITK_MANUAL_INSTANTIATION
#include "itk_otsu_threshold_image_calculator.txx"
#endif