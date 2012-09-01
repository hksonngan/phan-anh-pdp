#include <vector>

template <class PixelType>
pdp::visualizeData<PixelType>::visualizeData(QString info)
    : info(info)
{ }
template <class PixelType>
QString pdp::visualizeData<PixelType>::name() const
{
    return "visualizeData step: " + info;
}
template <class PixelType>
float pdp::visualizeData<PixelType>::relativePercent() const
{
    return 1.0;
}
template <class PixelType>
void pdp::visualizeData<PixelType>::work(LungDataset& input, LungDataset& output)
{
    std::cout << "performing the visualizeData step!" << std::endl;
    
	itk::Image<PixelType, 3>::Pointer itkVisData = createImage(input,0,0,1,0,0);
	emit stepProgress(0.1f);
	mitk::Image::Pointer mitkVisData = mitk::Image::New();
	mitk::CastToMitkImage(itkVisData, mitkVisData);
	output.addImage(mitkVisData, "VisData"); 
    emit stepProgress(1.00f);
}
template <class PixelType>
typename itk::Image<PixelType, 3>::Pointer pdp::visualizeData<PixelType>::createImage(LungDataset& input, bool lungContours = 1, bool convexHull = 1, bool thickenings = 1, bool bboxes = 1, bool thicks3d = 1)
{
	typedef itk::PolyLineParametricPath< 2 >::Pointer PathPointer;
	pdpItkSliceExtractor<PixelType> extractor("Creating output");
	extractor.setInputStepName("Lungs");
	extractor.setPathToImageParam(input);
	extractor.sliceList.clear();
	for (int i = 0; i < input.slicesContours.size(); ++i)
	{	
		std::vector<PathPointer> paths;
		if(input.slicesContours[i].isLeftLung()){
			ContourFilter contourFilterLeft(input.slicesContours[i].getLeftLung());
			if(lungContours)
				paths.push_back(input.slicesContours[i].getLeftLung());
			if(convexHull)
				paths.push_back(input.slicesContours[i].getLeftHull());
			for (int j = 0; j < input.slicesThickenings[i].sizeLeft(); j++)
			{
				if(bboxes){
					PathPointer boundingBoxL = ContourFilter::cgalToItk(input.slicesThickenings[i].getLeftThickening(j).getBoundingBox());			
					paths.push_back(boundingBoxL);
				}
				if(thickenings){
					PathPointer boundarL = ContourFilter::cgalToItk(input.slicesThickenings[i].getLeftThickening(j).getBoundary());
					paths.push_back (boundarL);
				}
			}
		}
		if(input.slicesContours[i].isRightLung()){
			ContourFilter contourFilterRight(input.slicesContours[i].getRightLung());
			if(lungContours)
				paths.push_back(input.slicesContours[i].getRightLung());
			if(convexHull)
				paths.push_back(input.slicesContours[i].getRightHull());
			for (int j = 0; j < input.slicesThickenings[i].sizeRight(); j++)
			{
				if(bboxes){
					PathPointer boundingBoxR = ContourFilter::cgalToItk(input.slicesThickenings[i].getRightThickening(j).getBoundingBox());			
					paths.push_back(boundingBoxR);
				}
				if(thickenings){
					PathPointer boundarR = ContourFilter::cgalToItk(input.slicesThickenings[i].getRightThickening(j).getBoundary());
					paths.push_back (boundarR);
				}
			}
		}
		if(thicks3d){
			for(int q=0; q<input.thickenings3d.size(); q++)
			for(int k=0; k<input.thickenings3d[q].sizeLeft(); k++){
				if(input.thickenings3d[q].getLeftThickening(k).getSliceNum()==i)
					paths.push_back(ContourFilter::cgalToItk(input.thickenings3d[q].getLeftThickening(k).getBoundary()));
			}
		}
		emit stepProgress(0.1f+0.8f/input.slicesContours.size()*i);
		extractor.sliceList.push_back(extractor.manualPathToImage(paths));
	}
	for(int k=0; k<input.thickenings3d[3].sizeLeft(); k++){
	
	}

	itk::Image<PixelType, 3>::Pointer itkRes = extractor.assembleSlice();
	return itkRes;
}
