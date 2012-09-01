#pragma once

#include "app/itk_filter_progress_watcher.h"

class cont{
public:
	cont(){
		left=1000000;
		right=0;
		top=0;
		bottom=1000000;
		size=0;
		centerX=0;
		centerY=0;
	}	
	int getLeft(){
		return left;
	}
	int getRight(){
		return right;
	}
	int getTop(){
		return top;
	}
	int getBottom(){
		return bottom;
	}
	float getCenterX(){
		return centerX;
	}
	float getCenterY(){
		return centerY;
	}
	int getSize(){
		return size;
	}
	void setLeft(int d){
		left=d;
	}
	void setRight(int d){
		right=d;
	}
	void setTop(int d){
		top=d;
	}
	void setBottom(int d){
		bottom=d;
	}
	void setSize(int d){
		size=d;
	}
	void addCenterX(int d){
		centerX+=(float)d;
	}
	void addCenterY(int d){
		centerY+=(float)d;
	}
	void averageCenters(int num){
		centerX/=(float)num;
		centerY/=(float)num;
	}
	void countSize(){
		size=(right-left)*(top-bottom);
	}

private:
	int left;
	int right;
	int top;
	int bottom;
	int size;
	float centerX;
	float centerY;

};
template <class PixelType>
pdp::contourExtractor<PixelType>::contourExtractor(QString info)
    : info(info)
{ 
for(int i=0; i<2;i++){
		boxBottom[i][0]=0;
		boxUp[i][0]=0;
		boxBottom[i][1]=0;
		boxUp[i][1]=0;
		boxBottom[i][2]=1000000;
		boxUp[i][2]=1000000;
		boxBottom[i][3]=1000000;
		boxUp[i][3]=1000000;
	}
//	regionIndex=new ImageType::IndexType();

}
template <class PixelType>
QString pdp::contourExtractor<PixelType>::name() const
{
    return "contourExtractor step: " + info;
}
template <class PixelType>
float pdp::contourExtractor<PixelType>::relativePercent() const
{
    return 1.0;
}
template <class PixelType>
void pdp::contourExtractor<PixelType>::work(LungDataset& input, LungDataset& output)
{
    std::cout << "performing the contourExtractor step!" << std::endl;	
	
	mitk::Image::Pointer mitkImage = input.getImage(inputStepName);
	mitkImage->DisconnectPipeline();

	itk::Image<PixelType, 3>::Pointer img;
	mitk::CastToItkImage(mitkImage, img);

	Contour(img,input);
	std::cout << "demoContour done "<< std::endl;

    emit stepProgress(1.00f);
}

template <class PixelType>
void pdp::contourExtractor<PixelType>::Contour(const typename itk::Image<PixelType, 3>::Pointer img, LungDataset& input)
{
	//data preparation
	//extractSlice(img);
	pdpItkSliceExtractor<PixelType> extractor("extracting contours");
	extractor.extractSlice(img);

	int middleUp=(float)extractor.getNumberOfSlices()*(1-(float)((float)middlePercentage/100));
	int middleBottom=(float)extractor.getNumberOfSlices()*(float)((float)middlePercentage/100);
	for(int i=0; i<extractor.getNumberOfSlices(); i++){
		input.slicesContours.push_back(sliceContour());
		input.slicesThickenings.push_back(sliceThickenings());
	}
	

	//data processing; in extractContour: part: 0 - bottom, 1 - middle, 2-up
	//first middle part of slices //in extractContour: firstOrLast: 0-middle or other parts, 1-first(bottom of the middle part), 2-up
	
	for (int i = middleBottom; i < middleUp; ++i)
	{	
		std::cout << "slice:   "<<i<<"/ "<<extractor.getNumberOfSlices()<< std::endl;
		if(i==middleBottom){
			extractContour(extractor.getSlice(i), 1, 1, input.slicesContours[i]);
		}
		else if(i==middleUp-1){
			extractContour(extractor.getSlice(i), 1, 2, input.slicesContours[i]);
		}
		else
			extractContour(extractor.getSlice(i), 1, 0, input.slicesContours[i]);   
    	emit stepProgress((float)(i-middleBottom) / (float)extractor.getNumberOfSlices() );
		
	}
	//then middle to the bottom
	for (int i = middleBottom - 1; i >=0; --i)
	{	
		std::cout << "slice:   "<<i<<"/ "<<extractor.getNumberOfSlices()<< std::endl;
		extractContour(extractor.getSlice(i), 0, 0, input.slicesContours[i]);
    	emit stepProgress((float)(middleUp-middleBottom -(i-middleBottom+1) )/ (float)extractor.getNumberOfSlices() );
		
	}
	//and up
	//char c;
	for (int i = middleUp; i < extractor.getNumberOfSlices(); ++i)
	{	
		//std::cin>>c;
		std::cout << "slice:   "<<i<<"/ "<<extractor.getNumberOfSlices()<< std::endl;
		extractContour(extractor.getSlice(i), 2, 0, input.slicesContours[i]);
		//pathList.push_back(path);
    	emit stepProgress((float)i / (float)extractor.getNumberOfSlices() );
		
	}
}

/**
*extractContour
*This (too) complex function extracts and separates left and right lung contour. It acts in different ways for different slices.
*Basically, first it chooses 10 biggest contours and computes statistics (size, bounding box, center) of each of them. 
*At this stage, it also eliminates contours touching the border or the big contour (which exists only in some images) bounding the 2 lungs.
*For middle slices, it chooses two biggest contours, for top and bottom it also takes the bounding box and the contours' center under consideration.
*It automatically adds the left and right lung into right places of the application.
*/
//the marching square contour
template <class PixelType>
void pdp::contourExtractor<PixelType>::extractContour(typename itk::Image<PixelType, 2>::Pointer img, int part, int firstOrLast,  sliceContour& sliceCont)
{
	typedef itk::Image<PixelType, 2> ImageType;

	//init contourExtractor2D
	typedef itk::ContourExtractor2DImageFilter <itk::Image<PixelType, 2>>
		ContourExtractor2DImageFilterType;
	ContourExtractor2DImageFilterType::Pointer contourExtractor2DImageFilter
		= ContourExtractor2DImageFilterType::New();
	contourExtractor2DImageFilter->SetContourValue(1);
	contourExtractor2DImageFilter->SetInput(img);
	contourExtractor2DImageFilter->Update();

	
	int maxItmp[]={-1, -1}; //neede for bottom and top parts
	int tmp=0;	//same
    int imgX=img->GetLargestPossibleRegion().GetSize()[0];
	int imgY=img->GetLargestPossibleRegion().GetSize()[1];
	int contourNum=0; //to know which contour we're operating on after ereasing some of them
    int max[maxNum]; //max[maxNum] has the biggest contour
    int maxIndex[maxNum];
    int cSize=0; //current size of the contour

	std::vector<cont> contours; //some statistics about each found contour - box, size, etc.
    typedef itk::PolyLineParametricPath< 2 >::Pointer PathPointer;
    std::vector<PathPointer> paths;

    for(int k = 0; k < maxNum; k++){
        maxIndex[k]=0;
        max[k]=0;
    }

    //choosing maxNum biggest contours
	for(int j = 0; j < contourExtractor2DImageFilter->GetNumberOfOutputs(); j++){
        cSize=contourExtractor2DImageFilter->GetOutput(j)->GetVertexList()->size();
        for(int k = 0; k < maxNum; k++){
                if (cSize > max[k])
		        {
                    if(k==0){
			            max[k] = cSize;
                        maxIndex[k] = j;
                    }
                    else{
                        max[k-1] = max[k];
                        maxIndex[k-1] = maxIndex[k];
                        max[k] = cSize;
                        maxIndex[k] = j;
                    }
		        }
        }
	}
	//we'll be operating on paths vector
    for(int k = 0; k < maxNum; k++){
        paths.push_back(contourExtractor2DImageFilter->GetOutput(maxIndex[k]));
		contours.push_back(cont());
    }

	typedef itk::PolyLineParametricPath< 2 > PathType;
	typedef itk::Image<PixelType, 2>::IndexType IndexType;
    
	const PathType::VertexListType * vertexList;
	ImageType::PointType point;
	ImageType::PointType pointm;
	IndexType index; //index of a particular vertex
	IndexType indexm;
	
	bool out=0; //if the countour was ereased

	//interating through maxNum biggest contours 
    for(int k = 0; k < maxNum; k++){
		out=false;
		vertexList=paths[contourNum]->GetVertexList();

        for(int l = 0; l < vertexList->size(); l++){ //interating through all vertices of the particular contour 
			img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(l), point);
			img->TransformPhysicalPointToIndex(point, index);

			//finding contours on the border
			if(index[0]<border || index[0]>imgX-border || //if 'touching' the border -> out
				index[1]<border || index[1]>imgY-border ){
				paths.erase(paths.begin()+contourNum);
				contours.erase(contours.begin()+contourNum);
				contourNum--;
				out=true;
				break;
			}
			
			else{ //updating centers
				contours[contourNum].addCenterX(index[0]);
				contours[contourNum].addCenterY(index[1]);
			}
			//finding the box of the contour
			if(!out){
				if(index[0]<contours[contourNum].getLeft())
					contours[contourNum].setLeft(index[0]);
				if(index[0]>contours[contourNum].getRight())
					contours[contourNum].setRight(index[0]);
				if(index[1]<contours[contourNum].getBottom())
					contours[contourNum].setBottom(index[1]);
				if(index[1]>contours[contourNum].getTop())
					contours[contourNum].setTop(index[1]);
			}

        }
		if(!out&&contours[contourNum].getRight()-contours[contourNum].getLeft()>imgX*0.9){//eliminating the 'big' contour if the image has a wide view 
			paths.erase(paths.begin()+contourNum);
			contours.erase(contours.begin()+contourNum);
			contourNum--;
			out=true;
		}
		
		if(!out){//counting centers and size of the contour
			contours[contourNum].averageCenters(vertexList->size());
			contours[contourNum].countSize();
		}
		contourNum++;
    }

	//std::cout<<"BEFORE REMOVING BOTTLENECK "<<contourNum<<std::endl;
	//for(int i=0;i<contourNum;i++)
	//	std::cout<<contours[i].getSize();
	//std::cout<<endl;
	//removing constriction problems
	int m_top=0;
	int m_bottom=0;
	max[0]=0;
	max[1]=0;
	max[2]=0;//biggest
	maxIndex[0]=0;
	maxIndex[1]=0;
	maxIndex[2]=0;
	float eucl_dist=0;
	float cont_dist=0;
	int tempX=0;
	int tempY=0;
	int contLenght=0;
	int bigger=0, smaller=0;
	IndexType most_bottom, most_top;
	most_bottom[0]=0;
	most_bottom[1]=0;
	most_top[0]=0;
	most_top[1]=0;
	int shortest=1000000;
	//only for 3 biggest contours
	for(int i = 0; i < contourNum; i++){
			cSize = contours[i].getSize();
			if(cSize>max[2]){
				max[1]=max[2];
				maxIndex[1]=maxIndex[2];
				max[2]=cSize;
				maxIndex[2]=i;
			}
			else if(cSize>max[1] && cSize<=max[2]){
				max[0]=max[1];
				maxIndex[0]=maxIndex[1];
				max[1]=cSize;
				maxIndex[1]=i;
			}
			else if(cSize>max[0] && cSize<=max[1]){
				max[0]=cSize;
				maxIndex[0]=i;
			}
	}

	for(int i = 0; i < contourNum; i++){//iterating through 3 cont.
		if(i==maxIndex[0] || i==maxIndex[1] || i==maxIndex[2]){
			if(contours[i].getRight()-contours[i].getLeft()<imgX*0.5)continue;
			shortest=1000000;
			//std::cout<<"big oone ";
			contLenght=0;
			vertexList=paths[i]->GetVertexList();
			cSize = vertexList->size();
			for(int l = 0; l < cSize; l++){ //interating through all vertices of the particular contour
				img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(l), point);
				img->TransformPhysicalPointToIndex(point, index);
				if(index[0]>imgX*0.7 || index[0]<imgX*0.3) continue; //only center part of the slice checked
				for(int m = 0; m < cSize; m++){
					if(m!=l){
						
						cont_dist=0;
						tempX=0;
						tempY=0;
						img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(m), pointm);
						img->TransformPhysicalPointToIndex(pointm, indexm);
						eucl_dist = sqrt((float)((index[0]-indexm[0])*(index[0]-indexm[0]) + (index[1]-indexm[1])*(index[1]-indexm[1])));
						if(eucl_dist>15) continue;
						if(indexm[0]>index[0]+index[0]*0.15 || indexm[0]<index[0]-index[0]*0.15) continue;
						//std::cout<<"NEXT"<<std::endl;
						/*if(m>l){
							for(int n = l; n <= m; n++){
								std::cout<<"cont "<<cont_dist<<std::endl;
								img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(n), pointm);
								img->TransformPhysicalPointToIndex(pointm, indexm);
								if(n==l){
									tempX=indexm[0];
									tempY=indexm[1];
									continue;
								}
								cont_dist+=sqrt((float)((tempX-indexm[0])*(tempX-indexm[0]) + (tempY-indexm[1])*(tempY-indexm[1])));
								tempX=indexm[0];
								tempY=indexm[1];
							}
						}
						else{
							for(int n = m; n <= l; n++){
								//std::cout<<"cont "<<cont_dist<<std::endl;
								img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(n), pointm);
								img->TransformPhysicalPointToIndex(pointm, indexm);
								if(n==l){
									tempX=indexm[0];
									tempY=indexm[1];
									continue;
								}
								//std::cout<<std::endl;
								//std::cout<<tempX<<" "<<indexm[0]<<" "<<tempY<<" "<<indexm[1]<<std::endl;
								//std::cout<<std::endl;
								cont_dist+=sqrt((float)((tempX-indexm[0])*(tempX-indexm[0]) + (tempY-indexm[1])*(tempY-indexm[1])));
								tempX=indexm[0];
								tempY=indexm[1];
							}
						}*/
						if(l>m){
							bigger=l;
							smaller=m;
						}
						else{
							bigger=m;
							smaller=l;
						}
						if(bigger-smaller<(cSize-bigger)+smaller)
							cont_dist=bigger-smaller;
						else
							cont_dist=(cSize-bigger)+smaller;
						//if(cont_dist>0)std::cout<<"cont "<<cont_dist<<std::endl;
						if(cont_dist > cSize*0.25){
							if(eucl_dist<shortest){
								shortest=eucl_dist;
								if(index[1]>indexm[1]){
									most_top[1]=index[1];
									most_bottom[1]=indexm[1];
									most_top[0]=index[0];
									most_bottom[0]=indexm[0];
									m_top=l;
									m_bottom=m;
								}
								else{
									most_top[1]=indexm[1];
									most_bottom[1]=index[1];
									most_top[0]=indexm[0];
									most_bottom[0]=index[0];
									m_top=m;
									m_bottom=l;
								}
							
							//std::cout<<"shortest:"<<shortest<<std::endl;
							//std::cout<<std::endl<<most_top[0]<<" moooostu "<<most_top[1];
							//std::cout<<std::endl<<most_bottom[0]<<" moooostu "<<most_bottom[1];
							}
							//std::cout<<"size "<<contLenght<<"e: "<<eucl_dist<<" "<<cont_dist<<" m "<<m<< " l "<<l<<std::endl;
						}
					}
				}
			}
			//std::cout<<std::endl<<most_top[0]<<"moooost"<<most_top[1]<<std::endl;
			//std::cout<<std::endl<<most_bottom[0]<<"moooost"<<most_bottom[1]<<std::endl;
			//add line here

			most_top[0]=(most_top[0]+most_bottom[0])/2;
			most_top[1]=(most_top[1]+most_bottom[1])/2;
			
			ContourFilter contourFilterL(paths[i]);
			ContourFilter contourFilterR(paths[i]);
			//UPDATIN STATISTICS
			//interating through maxNum+ 2 new biggest contours 
			paths.erase(paths.begin()+i);
			contours.erase(contours.begin()+i);
			//std::cout<<"before separation";
			paths.push_back(contourFilterL.separateLungs(1, m_top, m_bottom ));
			//std::cout<<"before right";
			paths.push_back(contourFilterR.separateLungs(0, m_top, m_bottom ));
			//std::cout<<"after separation";
			contours.push_back(cont());
			contours.push_back(cont());
			contourNum++;
				for(int k = 1; k <= 2; k++){
					vertexList=paths[contourNum-k]->GetVertexList();
					//std::cout<<"k"<<k<<" ";
					//std::cout<<vertexList->size()<<" ";
					for(int l = 0; l < vertexList->size(); l++){ //interating through all vertices of the particular contour 
						
						img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(l), point);
						img->TransformPhysicalPointToIndex(point, index);

						
							contours[contourNum-k].addCenterX(index[0]);
							contours[contourNum-k].addCenterY(index[1]);
						
						//finding the box of the contour
						
							if(index[0]<contours[contourNum-k].getLeft())
								contours[contourNum-k].setLeft(index[0]);
							if(index[0]>contours[contourNum-k].getRight())
								contours[contourNum-k].setRight(index[0]);
							if(index[1]<contours[contourNum-k].getBottom())
								contours[contourNum-k].setBottom(index[1]);
							if(index[1]>contours[contourNum-k].getTop())
								contours[contourNum-k].setTop(index[1]);	

					}
					
					//counting centers and size of the contour
						contours[contourNum-k].averageCenters(vertexList->size());
						contours[contourNum-k].countSize();
						//std::cout<<"after centers";
					
				}

		}
	}
	//end of removing contraints problem
	//std::cout<<"AFTER REMOVING BOTTLENECK "<<contourNum<<std::endl;
	//for(int i=0;i<contourNum;i++)
	//	std::cout<<contours[i].getSize();
	//std::cout<<endl;
	std::vector<PathPointer> pathsLR; //there will be left and next right contours
	max[0]=0;
	max[1]=0;
	max[2]=0;//biggest
	maxIndex[0]=0;
	maxIndex[1]=0;


	int indexLR[2];
	indexLR[0]=-1;
	indexLR[1]=-1;

	//*******MIDDLE PART
	//for the middle part: choosing 2 biggest contours
	if(part==1){
	
		//if we have to compare a contour with the previous one also in the middle part

		//int cn=contourNum;
		//contourNum=0;
		//if(firstOrLast != 1){
		//	for(int i = 0; i < cn; i++){
		//		
		//		if( //if out of the box, then out
		//			(contours[contourNum].getCenterX()<boxUp[0][3] && contours[contourNum].getCenterX()>boxUp[0][1] && 
		//			contours[contourNum].getCenterY()<boxUp[0][2] &&	contours[contourNum].getCenterY()>boxUp[0][0]) &&
		//			(contours[contourNum].getCenterX()<boxUp[1][3] && contours[contourNum].getCenterX()>boxUp[1][1] && 
		//			contours[contourNum].getCenterY()<boxUp[1][2] &&	contours[contourNum].getCenterY()>boxUp[1][0])
		//			)
		//		{
		//			paths.erase(paths.begin()+contourNum);
		//			contours.erase(contours.begin()+contourNum);
		//			contourNum--;
		//		}
		//		contourNum++;
		//	}
		//}


		//finding 2 biggest
		for(int i = 0; i < contourNum; i++){
			cSize = contours[i].getSize();
			if(cSize>max[1]){
				max[0]=max[1];
				maxIndex[0]=maxIndex[1];
				max[1]=cSize;
				maxIndex[1]=i;
			}
			else if(cSize>max[0] && cSize<=max[1]){
				max[0]=cSize;
				maxIndex[0]=i;
			}
		}
		//std::cout<<max[0]<<" "<<max[1];
		if(contours[maxIndex[1]].getCenterX()>contours[maxIndex[0]].getCenterX()){ //then 1 is right
			sliceCont.setLeftLung(paths[maxIndex[0]]);	//then 0 is right
			sliceCont.setRightLung(paths[maxIndex[1]]);
			indexLR[1]=maxIndex[1];
			indexLR[0]=maxIndex[0];
		}
		else{
			sliceCont.setLeftLung(paths[maxIndex[1]]);	//then 0 is right
			sliceCont.setRightLung(paths[maxIndex[0]]);
			indexLR[1]=maxIndex[0];
			indexLR[0]=maxIndex[1];
		}
		for(int k = 0; k < 2; k++) //setting the left and right box for first and last (maybe each) slice
		{
			if(firstOrLast==1){
				boxBottom[k][3]=contours[indexLR[k]].getLeft();
				boxBottom[k][1]=contours[indexLR[k]].getRight();
				boxBottom[k][2]=contours[indexLR[k]].getBottom();
				boxBottom[k][0]=contours[indexLR[k]].getTop();	
			}
			if(firstOrLast==2){//!!if we have to compare a contour with the previous one also in the middle part ->then remove 'if'
				boxUp[k][3]=contours[indexLR[k]].getLeft();
				boxUp[k][1]=contours[indexLR[k]].getRight();
				boxUp[k][2]=contours[indexLR[k]].getBottom();
				boxUp[k][0]=contours[indexLR[k]].getTop();
				//std::cout<<"SETTTING TOP!!!!!!!!!!!!!!!"<<std::endl;
				//std::cout<<"BOXL:left "<<boxUp[0][3]<<" right "<<boxUp[0][1]<<" top "<<boxUp[0][0]<<" bottom"<< boxUp[0][2]<<std::endl;
				//std::cout<<"BOXR:left "<<boxUp[1][3]<<" right "<<boxUp[1][1]<<" top "<<boxUp[1][0]<<" bottom"<< boxUp[1][2]<<std::endl;
			}
		}
		//we have convex hull so do we need it???
		//REMOVING BRONCHUS
		//std::cout<<"bronchus";

		//int v=0;
		//
		//int tmp=0;
		//int compare=0;
		//bool way=false; //0 = decrease ,1 = increase
		////right first
		//vertexList=paths[indexLR[1]]->GetVertexList();
		//cSize = vertexList->size();
		//for(int k=0; k<2; k++)
		//for(int l = 0; l < cSize; l++){ //interating through all vertices of the particular contour
		//	img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(l), point);
		//	img->TransformPhysicalPointToIndex(point, index);
		//	if(k==0)
		//		compare=contours[indexLR[1]].getTop();
		//	else
		//		compare=contours[indexLR[1]].getBottom();
		//	if(index[1]==compare){
		//		v=l;
		//		tmp=v;
		//		v++;
		//		if(v==cSize)
		//			img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(0), pointm);
		//		else
		//			img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(v), pointm);
		//		img->TransformPhysicalPointToIndex(pointm, indexm);
		//		while(indexm[0]==index[0]){
		//			v++;
		//			std::cout<<v;
		//			if(v==cSize){
		//				v=0;
		//			}
		//			img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(v), pointm);
		//			img->TransformPhysicalPointToIndex(pointm, indexm);
		//		}
		//		if(indexm[0]<index[0]) //ok
		//			way=true;
		//		else
		//			way=false;
		//		
		//		tempX=index[0];
		//		
		//		while(true){
		//			if(way){
		//				v+=2;
		//				if(v>=cSize)v=0;
		//			}
		//			else {
		//				v-=2;
		//				if(v<=-1)v=cSize-1;
		//			}
		//			
		//			img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(v), point);
		//			img->TransformPhysicalPointToIndex(point, index);
		//			if(tempX<index[0]){
		//				if(k==0){
		//					most_top[0]=index[0];
		//					most_top[1]=index[1];
		//					m_top=v;
		//				}
		//				else{
		//					most_bottom[0]=index[0];
		//					most_bottom[1]=index[1];
		//					m_bottom=v;
		//				}
		//				break;
		//			}
		//			tempX=index[0];
		//		}
		//		break;
		//	}
		//}
		////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		////we have now 2 points, and we erase all between them - dont know if the following method works
		////to be resolved if its really needed
		////also need to modify itk files to allow removing the vertices or use cgal
		//int r=0;
		//if(m_top>m_bottom){
		//					bigger=m_top;
		//					smaller=m_bottom;
		//				}
		//				else{
		//					bigger=m_bottom;
		//					smaller=m_top;
		//				}
		//				if(bigger-smaller<(cSize-bigger)+smaller)
		//					for(int i=bigger;i>smaller;i--){
		//						paths[indexLR[0]]->DeleteVertex(i);
		//					}
		//				else{

		//					while(true){
		//						r=bigger;
		//						paths[indexLR[0]]->DeleteVertex(r);
		//						if(r==smaller)break;
		//						if(r==cSize)r=0;
		//						r++;
		//					}
		//				}
		///*if(m_top>m_bottom)
		//	for(int i=m_top;i>m_bottom;i--){
		//		paths[indexLR[0]]->DeleteVertex(i);
		//	}
		//else
		//	for(int i=m_bottom;i>m_top;i--){
		//		paths[indexLR[0]]->DeleteVertex(i);
		//	}*/
		////end of right
		//v=0;
		//tmp=0;
		//compare=0;
		//way=false; //0 = decrease ,1 = increase
		////now left
		//vertexList=paths[indexLR[0]]->GetVertexList();
		//cSize = vertexList->size();
		//for(int k=0; k<2; k++)
		//for(int l = 0; l < cSize; l++){ //interating through all vertices of the particular contour
		//	img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(l), point);
		//	img->TransformPhysicalPointToIndex(point, index);
		//	if(k==0)
		//		compare=contours[indexLR[0]].getTop();
		//	else
		//		compare=contours[indexLR[0]].getBottom();
		//	if(index[1]==compare){
		//		v=l;
		//		tmp=v;
		//		v++;
		//		if(v==cSize)
		//			img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(0), pointm);
		//		else
		//			img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(v), pointm);
		//		img->TransformPhysicalPointToIndex(pointm, indexm);
		//		while(indexm[0]==index[0]){
		//			v++;
		//			std::cout<<v;
		//			if(v==cSize){
		//				v=0;
		//			}
		//			img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(v), pointm);
		//			img->TransformPhysicalPointToIndex(pointm, indexm);
		//		}
		//		if(indexm[0]<index[0]) //not ok
		//			way=true;
		//		else
		//			way=false;
		//		
		//		tempX=index[0];
		//		
		//		while(true){
		//			if(!way){
		//				v+=2;
		//				if(v>=cSize)v=0;
		//			}
		//			else {
		//				v-=2;
		//				if(v<=-1)v=cSize-1;
		//			}
		//			
		//			img->TransformContinuousIndexToPhysicalPoint(vertexList->GetElement(v), point);
		//			img->TransformPhysicalPointToIndex(point, index);
		//			if(tempX>index[0]){
		//				if(k==0){
		//					most_top[0]=index[0];
		//					most_top[1]=index[1];
		//					m_top=v;
		//				}
		//				else{
		//					most_bottom[0]=index[0];
		//					most_bottom[1]=index[1];
		//					m_bottom=v;
		//				}
		//				break;
		//			}
		//			tempX=index[0];
		//		}
		//		break;
		//	}
		//}
		//int am=paths[indexLR[0]]->GetVertexList()->size();
		//
		//if(m_top>m_bottom)
		//	for(int i=m_top;i>m_bottom;i--){
		//		paths[indexLR[0]]->DeleteVertex(i);
		//	}
		//else
		//	for(int i=m_bottom;i>m_top;i--){
		//		paths[indexLR[0]]->DeleteVertex(i);
		//	}
	//END OF REMOVING BRONCHIAL TREE
	}

	
	

	//******BOTTOM PART
	//for the bottom part - choosing 3 biggest, then checking centers, and updating the box
	else if(part==0){
		maxItmp[0]= -1;
		maxItmp[1]= -1;
		max[0]=0;
		max[1]=0;
		max[2]=0;
		maxIndex[0]= -1;
		maxIndex[1]= -1;
		maxIndex[2]= -1;
		//std::cout<<"BOTTOM";
		indexLR[0]=-1;
		indexLR[1]=-1;
		for(int i = 0; i < contourNum; i++){
			cSize = contours[i].getSize();
			if(cSize>max[2]){
				max[1]=max[2];
				maxIndex[1]=maxIndex[2];
				max[2]=cSize;
				maxIndex[2]=i;
			}
			else if(cSize>max[1] && cSize<=max[2]){
				max[0]=max[1];
				maxIndex[0]=maxIndex[1];
				max[1]=cSize;
				maxIndex[1]=i;
			}
			else if(cSize>max[0] && cSize<=max[1]){
				max[0]=cSize;
				maxIndex[0]=i;
			}
		}
		//std::cout<<max[0]<<" "<<max[1]<<" "<<max[2]<<std::endl;
		for(int i = 0; i < contourNum; i++){
			if(i==maxIndex[0] || i==maxIndex[1] || i==maxIndex[2]){
				if(i==maxIndex[0])
					tmp=0;
				else if(i==maxIndex[1])
					tmp=1;
				else if(i==maxIndex[2])
					tmp=2;
				
				if(contours[i].getCenterX()>boxBottom[0][3] && contours[i].getCenterX()<boxBottom[0][1] && //center inside the bounding box
				contours[i].getCenterY()>boxBottom[0][2] &&	contours[i].getCenterY()<boxBottom[0][0] ) 
				{
					if( indexLR[0]==-1 || max[i]> max[indexLR[0]])//then it is left
						indexLR[0]=i;
				}
				if(contours[i].getCenterX()>boxBottom[1][3] && contours[i].getCenterX()<boxBottom[1][1] && //center inside the bounding box
				contours[i].getCenterY()>boxBottom[1][2] &&	contours[i].getCenterY()<boxBottom[1][0] ) 
				{
					if(indexLR[1]==-1 || max[i]> max[indexLR[1]] )//then it is right
						indexLR[1]=i;
				}
			}
		}
		for(int k = 0; k < 2; k++){
			if(k==0 && indexLR[0] == -1)
				break;
			if(k==1 && indexLR[1] == -1)
				break;
			
			boxBottom[k][3]=contours[indexLR[k]].getLeft();
			boxBottom[k][1]=contours[indexLR[k]].getRight();
			boxBottom[k][2]=contours[indexLR[k]].getBottom();
			boxBottom[k][0]=contours[indexLR[k]].getTop();
			if(k==0)
				sliceCont.setLeftLung(paths[indexLR[0]]);
			if(k==1)
				sliceCont.setRightLung(paths[indexLR[1]]);
		}	
	}

	//TOP PART
	//the same as for the bottom
	else if(part==2){
		maxItmp[0]= -1;
		maxItmp[1]= -1;
		indexLR[0]=-1;
		indexLR[1]=-1;
		max[0]=0;
		max[1]=0;
		max[2]=0;
		maxIndex[0]= -1;
		maxIndex[1]= -1;
		maxIndex[2]= -1;
		for(int i = 0; i < contourNum; i++){
			cSize = contours[i].getSize();

			if(cSize>max[2]){
				max[1]=max[2];
				maxIndex[1]=maxIndex[2];
				max[2]=cSize;
				maxIndex[2]=i;
			}
			else if(cSize>max[1] && cSize<=max[2]){
				max[0]=max[1];
				maxIndex[0]=maxIndex[1];
				max[1]=cSize;
				maxIndex[1]=i;
			}
			else if(cSize>max[0] && cSize<=max[1]){
				max[0]=cSize;
				maxIndex[0]=i;
			}
		}
		//std::cout<<max[0]<<" "<<max[1]<<" "<<max[2]<<std::endl;
		//std::cout<<max[0]<<" "<<max[1]<<" "<<max[2]<<std::endl;
		for(int i = 0; i < contourNum; i++){
			if(i==maxIndex[0] || i==maxIndex[1] || i==maxIndex[2]){
				if(i==maxIndex[0])
					tmp=0;
				else if(i==maxIndex[1])
					tmp=1;
				else if(i==maxIndex[2])
					tmp=2;
				//std::cout<<std::endl;
				//std::cout<<std::cout<<"BOXL:left "<<boxUp[0][3]<<" right "<<boxUp[0][1]<<" top "<<boxUp[0][0]<<" bottom"<< boxUp[0][2]<<std::endl;
				//std::cout<<"BOXR:left "<<boxUp[1][3]<<" right "<<boxUp[1][1]<<" top "<<boxUp[1][0]<<" bottom"<< boxUp[1][2]<<std::endl;
				//std::cout<<"Center "<<i<<": x "<<contours[i].getCenterX()<<" y "<<contours[i].getCenterY()<<"Size: max[tmp] "<<max[tmp]<<std::endl;

				if(contours[i].getCenterX()>boxUp[0][3] && contours[i].getCenterX()<boxUp[0][1] && 
				contours[i].getCenterY()>boxUp[0][2] &&	contours[i].getCenterY()<boxUp[0][0] ) //then it is left
				{
					if( indexLR[0]== -1 || maxItmp[0]== -1 ||max[tmp]> max[maxItmp[0]]){
						indexLR[0]=i;
						maxItmp[0]=tmp;
					}
				}
				if(contours[i].getCenterX()>boxUp[1][3] && contours[i].getCenterX()<boxUp[1][1] && 
				contours[i].getCenterY()>boxUp[1][2] &&	contours[i].getCenterY()<boxUp[1][0] ) //then it is right
				{
					if(indexLR[1]==-1 || maxItmp[1]== -1 || max[tmp]> max[maxItmp[1]] ){
						indexLR[1]=i;
						maxItmp[1]=tmp;
						}
				}
				//std::cout<<"CHOSEN: "<<indexLR[0]<<" "<<indexLR[0];
			}
		}
		
		for(int k = 0; k < 2; k++){
			if(k==0 && indexLR[0] == -1)
				break;
			if(k==1 && indexLR[1] == -1)
				break;
			cSize=contours[indexLR[k]].getSize();
			
			boxUp[k][3]=contours[indexLR[k]].getLeft();
			boxUp[k][1]=contours[indexLR[k]].getRight();
			boxUp[k][2]=contours[indexLR[k]].getBottom();
			boxUp[k][0]=contours[indexLR[k]].getTop();
			
			if(k==0)
				sliceCont.setLeftLung(paths[indexLR[0]]);
			if(k==1)
				sliceCont.setRightLung(paths[indexLR[1]]);
		}	
	}


	std::vector<PathPointer>().swap(paths);
	std::vector<PathPointer>().swap(pathsLR);

}

