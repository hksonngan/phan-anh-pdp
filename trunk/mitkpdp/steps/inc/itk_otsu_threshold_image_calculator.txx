/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkOtsuThresholdImageCalculator.txx,v $
  Language:  C++
  Date:      $Date: 2009-01-26 21:45:54 $
  Version:   $Revision: 1.9 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#pragma once

#include <itkOtsuThresholdImageCalculator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkMinimumMaximumImageCalculator.h>

#include <vnl/vnl_math.h>

namespace itk
{ 
    
/**
 * Constructor
 */
template<class TInputImage>
OtsuThresholdImageCalculator<TInputImage>
::OtsuThresholdImageCalculator()
{
  m_Image = NULL;
  m_Threshold = NumericTraits<PixelType>::Zero;
  m_NumberOfHistogramBins = 256;
  m_RegionSetByUser = false;
  isMask = false;
}


/*
 * Compute the Otsu's threshold
 */
template<class TInputImage>
void
OtsuThresholdImageCalculator<TInputImage>
::Compute(void)
{

  unsigned int j;

  if ( !m_Image ) { return; }
  if( !m_RegionSetByUser )
    {
    m_Region = m_Image->GetRequestedRegion();
    }

  double totalPixels = (double) m_Region.GetNumberOfPixels();
  if ( totalPixels == 0 ) { return; }


  // compute image max and min
  typedef MinimumMaximumImageCalculator<TInputImage> RangeCalculator;
  typename RangeCalculator::Pointer rangeCalculator = RangeCalculator::New();
  rangeCalculator->SetImage( m_Image );
  rangeCalculator->Compute();

  PixelType imageMin = rangeCalculator->GetMinimum();
  PixelType imageMax = rangeCalculator->GetMaximum();

  if ( imageMin >= imageMax )
    {
    m_Threshold = imageMin;
    return;
    }

  // create a histogram
  std::vector<double> relativeFrequency;
  relativeFrequency.resize( m_NumberOfHistogramBins );
  for ( j = 0; j < m_NumberOfHistogramBins; j++ )
    {
    relativeFrequency[j] = 0.0;
    }

  double binMultiplier = (double) m_NumberOfHistogramBins /
    (double) ( imageMax - imageMin );

  typedef ImageRegionConstIteratorWithIndex<TInputImage> Iterator;
  Iterator iter( m_Image, m_Region );

  while ( !iter.IsAtEnd() )
    {
    unsigned int binNumber;
    PixelType value = iter.Get();

    if ( value == imageMin ) 
      {
      binNumber = 0;
      }
    else
      {
      binNumber = (unsigned int) vcl_ceil((value - imageMin) * binMultiplier ) - 1;
      if ( binNumber == m_NumberOfHistogramBins ) // in case of rounding errors
        {
        binNumber -= 1;
        }
      }

    relativeFrequency[binNumber] += 1.0;
    ++iter;

    }
 
  // normalize the frequencies
  double totalMean = 0.0;
  for ( j = 0; j < m_NumberOfHistogramBins; j++ )
    {
    relativeFrequency[j] /= totalPixels;
    totalMean += (j+1) * relativeFrequency[j];
    }


  // compute Otsu's threshold by maximizing the between-class
  // variance
  double freqLeft = relativeFrequency[0];
  double meanLeft = 1.0;
  double meanRight = ( totalMean - freqLeft ) / ( 1.0 - freqLeft );

  double maxVarBetween = freqLeft * ( 1.0 - freqLeft ) *
    vnl_math_sqr( meanLeft - meanRight );
  int maxBinNumber = 0;

  double freqLeftOld = freqLeft;
  double meanLeftOld = meanLeft;

  for ( j = 1; j < m_NumberOfHistogramBins; j++ )
    {
    freqLeft += relativeFrequency[j];
    meanLeft = ( meanLeftOld * freqLeftOld + 
                 (j+1) * relativeFrequency[j] ) / freqLeft;
    if (freqLeft == 1.0)
      {
      meanRight = 0.0;
      }
    else
      {
      meanRight = ( totalMean - meanLeft * freqLeft ) / 
        ( 1.0 - freqLeft );
      }
    double varBetween = freqLeft * ( 1.0 - freqLeft ) *
      vnl_math_sqr( meanLeft - meanRight );
   
    if ( varBetween > maxVarBetween )
      {
      maxVarBetween = varBetween;
      maxBinNumber = j;
      }

    // cache old values
    freqLeftOld = freqLeft;
    meanLeftOld = meanLeft; 

    } 

  m_Threshold = static_cast<PixelType>( imageMin + 
                                        ( maxBinNumber + 1 ) / binMultiplier );
}




template<class TInputImage>
void
OtsuThresholdImageCalculator<TInputImage>
::ComputeSVRC(void)
{

  unsigned int j;

  if ( !m_Image ) { return; }
  if( !m_RegionSetByUser )
    {
    m_Region = m_Image->GetRequestedRegion();
    }

  double totalPixels = (double) m_Region.GetNumberOfPixels();
  if ( totalPixels == 0 ) { return; }


  // compute image max and min
  typedef MinimumMaximumImageCalculator<TInputImage> RangeCalculator;
  typename RangeCalculator::Pointer rangeCalculator = RangeCalculator::New();
  rangeCalculator->SetImage( m_Image );
  rangeCalculator->Compute();

  PixelType imageMin = rangeCalculator->GetMinimum();
  PixelType imageMax = rangeCalculator->GetMaximum();

  if ( imageMin >= imageMax )
    {
    m_Threshold = imageMin;
    return;
    }

  // create a histogram
  std::vector<double> relativeFrequency;
  relativeFrequency.resize( m_NumberOfHistogramBins );
  for ( j = 0; j < m_NumberOfHistogramBins; j++ )
    {
    relativeFrequency[j] = 0.0;
    }

  double binMultiplier = (double) m_NumberOfHistogramBins /
    (double) ( imageMax - imageMin );

  typedef ImageRegionConstIteratorWithIndex<TInputImage> Iterator;
  Iterator iter( m_Image, m_Region );

  while ( !iter.IsAtEnd() )
    {
    unsigned int binNumber;
    PixelType value = iter.Get();

    if ( value == imageMin ) 
      {
      binNumber = 0;
      }
    else
      {
      binNumber = (unsigned int) vcl_ceil((value - imageMin) * binMultiplier ) - 1;
      if ( binNumber == m_NumberOfHistogramBins ) // in case of rounding errors
        {
        binNumber -= 1;
        }
      }

    relativeFrequency[binNumber] += 1.0;
    ++iter;

    }
 
  // normalize the frequencies, calculate cumulative histogram
  std::vector<double> cumulativeHistogram;
  cumulativeHistogram.resize( m_NumberOfHistogramBins );
  double totalMean = 0.0;

  bool lowerFound = false;
  bool upperFound = false;
  int LBIndex;
  int UBIndex;
  double meanLeft = 0.0;

  for ( j = 0; j < m_NumberOfHistogramBins; j++ )
  {
    relativeFrequency[j] /= totalPixels;
	totalMean += (j+1) * relativeFrequency[j];
	//for (int i = 0; i <= j; i++)
	//{
	//	cumulativeHistogram[j] += relativeFrequency[i];
	//}
	if (j == 0)
	{
		cumulativeHistogram[j] = relativeFrequency[j];
	}
	else
	{
		cumulativeHistogram[j] = cumulativeHistogram[j-1] + relativeFrequency[j];
	}

	  if (cumulativeHistogram[j] >= m_LowerBound && !lowerFound)
	  {
		  lowerFound = true;
		  LBIndex = j;
		  meanLeft = totalMean/cumulativeHistogram[j];
		  std::cout << "lower bin " << j << " cumulative " << cumulativeHistogram[j] << " threshold " << imageMin + ( j + 1 ) / binMultiplier << std::endl;
	  }

	  if (cumulativeHistogram[j] > m_UpperBound && !upperFound && j > 0 && lowerFound)
	  {
		  upperFound = true;
		  UBIndex = j-1;
		  std::cout << "upper bin " << j-1 << " cumulative " << cumulativeHistogram[j-1] << " threshold " << imageMin + ( j ) / binMultiplier << std::endl;
	  }

  }





  // compute Otsu's threshold by maximizing the between-class
  // variance
  double freqLeft = cumulativeHistogram[LBIndex];
  double meanRight = ( totalMean - meanLeft * freqLeft ) / ( 1.0 - freqLeft );

  double maxVarBetween = freqLeft * ( 1.0 - freqLeft ) *
    vnl_math_sqr( meanLeft - meanRight );
  int maxBinNumber = LBIndex;

  double freqLeftOld = freqLeft;
  double meanLeftOld = meanLeft;

  for ( j = LBIndex+1; j <= UBIndex; j++ )
    {
    freqLeft += relativeFrequency[j];
    meanLeft = ( meanLeftOld * freqLeftOld + 
                 (j+1) * relativeFrequency[j] ) / freqLeft;
    if (freqLeft == 1.0)
      {
      meanRight = 0.0;
      }
    else
      {
      meanRight = ( totalMean - meanLeft * freqLeft ) / 
        ( 1.0 - freqLeft );
      }
    double varBetween = freqLeft * ( 1.0 - freqLeft ) *
      vnl_math_sqr( meanLeft - meanRight );
   
    if ( varBetween > maxVarBetween )
      {
      maxVarBetween = varBetween;
      maxBinNumber = j;
      }

    // cache old values
    freqLeftOld = freqLeft;
    meanLeftOld = meanLeft; 

    } 

  m_Threshold = static_cast<PixelType>( imageMin + 
                                        ( maxBinNumber + 1 ) / binMultiplier );

  std::cout << "threshold " << m_Threshold << " max bin number " << maxBinNumber << std::endl;
}








template<class TInputImage>
void
OtsuThresholdImageCalculator<TInputImage>
::ComputeSVRCWithMask(void)
{

  unsigned int j;

  if ( !m_Image ) { return; }
  if( !m_RegionSetByUser )
    {
    m_Region = m_Image->GetRequestedRegion();
    }

  double totalPixels = (double) m_Region.GetNumberOfPixels();
  if ( totalPixels == 0 ) { return; }


  // compute image max and min
	MinMaxMask();

  PixelType imageMin = m_Minimum;
  PixelType imageMax = m_Maximum;

  if ( imageMin >= imageMax )
    {
    m_Threshold = imageMin;
    return;
    }

  // create a histogram
  std::vector<double> relativeFrequency;
  relativeFrequency.resize( m_NumberOfHistogramBins );
  for ( j = 0; j < m_NumberOfHistogramBins; j++ )
    {
    relativeFrequency[j] = 0.0;
    }

  double binMultiplier = (double) m_NumberOfHistogramBins /
    (double) ( imageMax - imageMin );

  typedef ImageRegionConstIteratorWithIndex<TInputImage> Iterator;
  Iterator iter( m_Image, m_Region );
  Iterator iterMask( m_Mask, m_Mask->GetRequestedRegion() );

  while ( !iter.IsAtEnd() )
  {
	if (itMask.Get() == 255)
	{
		unsigned int binNumber;
		PixelType value = iter.Get();

		if ( value == imageMin ) 
		  {
		  binNumber = 0;
		  }
		else
		  {
		  binNumber = (unsigned int) vcl_ceil((value - imageMin) * binMultiplier ) - 1;
		  if ( binNumber == m_NumberOfHistogramBins ) // in case of rounding errors
			{
			binNumber -= 1;
			}
		  }

		relativeFrequency[binNumber] += 1.0;
	}
	++iter;
	++iterMask;
  }
 
  // normalize the frequencies, calculate cumulative histogram
  std::vector<double> cumulativeHistogram;
  cumulativeHistogram.resize( m_NumberOfHistogramBins );
  double totalMean = 0.0;

  bool lowerFound = false;
  bool upperFound = false;
  int LBIndex;
  int UBIndex;
  double meanLeft = 0.0;

  for ( j = 0; j < m_NumberOfHistogramBins; j++ )
  {
    relativeFrequency[j] /= totalPixels;
	totalMean += (j+1) * relativeFrequency[j];
	//for (int i = 0; i <= j; i++)
	//{
	//	cumulativeHistogram[j] += relativeFrequency[i];
	//}
	if (j == 0)
	{
		cumulativeHistogram[j] = relativeFrequency[j];
	}
	else
	{
		cumulativeHistogram[j] = cumulativeHistogram[j-1] + relativeFrequency[j];
	}

	  if (cumulativeHistogram[j] >= m_LowerBound && !lowerFound)
	  {
		  lowerFound = true;
		  LBIndex = j;
		  meanLeft = totalMean/cumulativeHistogram[j];
		  std::cout << "lower bin " << j << " cumulative " << cumulativeHistogram[j] << " threshold " << imageMin + ( j + 1 ) / binMultiplier << std::endl;
	  }

	  if (cumulativeHistogram[j] > m_UpperBound && !upperFound && j > 0 && lowerFound)
	  {
		  upperFound = true;
		  UBIndex = j-1;
		  std::cout << "upper bin " << j-1 << " cumulative " << cumulativeHistogram[j-1] << " threshold " << imageMin + ( j ) / binMultiplier << std::endl;
	  }

  }





  // compute Otsu's threshold by maximizing the between-class
  // variance
  double freqLeft = cumulativeHistogram[LBIndex];
  double meanRight = ( totalMean - meanLeft * freqLeft ) / ( 1.0 - freqLeft );

  double maxVarBetween = freqLeft * ( 1.0 - freqLeft ) *
    vnl_math_sqr( meanLeft - meanRight );
  int maxBinNumber = LBIndex;

  double freqLeftOld = freqLeft;
  double meanLeftOld = meanLeft;

  for ( j = LBIndex+1; j <= UBIndex; j++ )
    {
    freqLeft += relativeFrequency[j];
    meanLeft = ( meanLeftOld * freqLeftOld + 
                 (j+1) * relativeFrequency[j] ) / freqLeft;
    if (freqLeft == 1.0)
      {
      meanRight = 0.0;
      }
    else
      {
      meanRight = ( totalMean - meanLeft * freqLeft ) / 
        ( 1.0 - freqLeft );
      }
    double varBetween = freqLeft * ( 1.0 - freqLeft ) *
      vnl_math_sqr( meanLeft - meanRight );
   
    if ( varBetween > maxVarBetween )
      {
      maxVarBetween = varBetween;
      maxBinNumber = j;
      }

    // cache old values
    freqLeftOld = freqLeft;
    meanLeftOld = meanLeft; 

    } 

  m_Threshold = static_cast<PixelType>( imageMin + 
                                        ( maxBinNumber + 1 ) / binMultiplier );

  std::cout << "threshold " << m_Threshold << " max bin number " << maxBinNumber << std::endl;
}










/**
 * Compute Min and Max of m_Image with mask
 */
template<class TInputImage>
void
OtsuThresholdImageCalculator<TInputImage>
::MinMaxMask(void)
{
  if( !m_RegionSetByUser )
    {
    m_Region = m_Image->GetRequestedRegion();
    }

  ImageRegionConstIteratorWithIndex< TInputImage >  it( m_Image, m_Region );
  ImageRegionConstIteratorWithIndex< TInputImage >  itMask( m_Mask, m_Mask->GetRequestedRegion() );

  m_Maximum = NumericTraits<PixelType>::NonpositiveMin();
  m_Minimum = NumericTraits<PixelType>::max();


  while( !it.IsAtEnd() )
  {
	if (itMask.Get() == 255)
	{
		const PixelType value = it.Get();  
		if (value > m_Maximum) 
		  {
		  m_Maximum = value;
		  m_IndexOfMaximum = it.GetIndex();
		  }
		if (value < m_Minimum) 
		  {
		  m_Minimum = value;
		  m_IndexOfMinimum = it.GetIndex();
		  }

	}
    ++it;
	++itMask;
  }

}





template<class TInputImage>
void
OtsuThresholdImageCalculator<TInputImage>
::SetRegion( const RegionType & region )
{
  m_Region = region;
  m_RegionSetByUser = true;
}

  
template<class TInputImage>
void
OtsuThresholdImageCalculator<TInputImage>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Threshold: " << m_Threshold << std::endl;
  os << indent << "NumberOfHistogramBins: " << m_NumberOfHistogramBins << std::endl;
  os << indent << "Image: " << m_Image.GetPointer() << std::endl;
}

} // end namespace itk
