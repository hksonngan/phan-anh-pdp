#ifndef StartStopChanVese_h_Included
#define StartStopChanVese_h_Included

#include "itkScalarChanAndVeseSparseLevelSetImageFilter.h"

namespace itk
{
/**
 * \class SNAPLevelSetStopAndGoFilter
 * \brief An extension of the ITK SparseFieldLevelSetImageFilter that allows
 * users to execute one iteration at a time.
 *
 * This class will no longer be necessary if the functionality is added to the
 * FiniteDifferenceImageFilter class in ITK.
 */
template <class TInputImage, class TFeatureImage, class TOutputImage, class TFunction, class TSharedData, typename TIdCell = unsigned int >
class StartStopChanVese : public itk::ScalarChanAndVeseSparseLevelSetImageFilter<TInputImage, TFeatureImage, TOutputImage, TFunction, TSharedData, TIdCell>
{
	public:
		/** Standard class typedefs. */
		typedef StartStopChanVese Self;
		typedef itk::ScalarChanAndVeseSparseLevelSetImageFilter<TInputImage, TFeatureImage, TOutputImage, TFunction, TSharedData, TIdCell> Superclass;
		typedef itk::SmartPointer<Self> Pointer;
		typedef itk::SmartPointer<const Self> ConstPointer;

		/** Some typedefs from the parent class */
		typedef typename Superclass::TimeStepType TimeStepType;

		/** Run-time type information. */
		itkTypeMacro(StartStopChanVese, itk::ScalarChanAndVeseSparseLevelSetImageFilter);

		/** New object of this type */
		itkNewMacro(StartStopChanVese);

		/** Initialize ScalarChanAndVeseSparseLevelSetImageFilter */
		//void Initialize();

		/** Initialize the filter before calling Run */
		void Start();

		/** Method that runs the filter for a number of iterations */
		void Run(unsigned int nIterations);

		/** Override the generate data method to do nothing */
		void GenerateData();

		void CallUpdate();

	protected:
		StartStopChanVese();
		~StartStopChanVese(){};
		void PrintSelf(std::ostream &s, itk::Indent indent) const;

		/** Dummy implementation.  Since the filter does nothing in Update(), this 
		* method should never be called anyway */
		virtual bool Halt() { std::cout << "Halt called.\n"; return true; };

	private:
		StartStopChanVese(const Self&); //purposely not implemented
		void operator=(const Self&); //purposely not implemented

		/** Whether or not the stop and go is going */
		bool     m_Started;
};
} //namespace

#ifndef ITK_MANUAL_INSTANTIATION
#include "StartStopChanVese.txx"
#endif

#endif // StartStopChanVese_h_Included
