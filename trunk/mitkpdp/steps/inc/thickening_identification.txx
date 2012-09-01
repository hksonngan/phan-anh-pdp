int singlethicks=0;

pdp::thickeningsIdentification::thickeningsIdentification(QString info)
    : info(info)
{ 
	thickRemovalSize = 200;
}

QString pdp::thickeningsIdentification::name() const
{
    return "thickeningsIdentification step: " + info;
}

float pdp::thickeningsIdentification::relativePercent() const
{
    return 1.0;
}

void pdp::thickeningsIdentification::work(LungDataset& input, LungDataset& output)
{
    std::cout << "performing the thickeningsIdentification step!" << std::endl;	
	//Eliminating big thicks
	int tmpnum=0;
	for(int i=0; i<input.slicesThickenings.size(); i++){ //iterating through all slices
			//LEFT LUNG
			if(input.slicesThickenings[i].isLeftThickenings()){
				tmpnum+=input.slicesThickenings[i].sizeLeft(); //<-testing purpose
				for(int j=0; j<input.slicesThickenings[i].sizeLeft(); j++){//iterating through all thickenigns on the slice, left lung
					std::cout<<input.slicesThickenings[i].getLeftThickening(j).getBoundary().area()<<" ";
					if(abs(input.slicesThickenings[i].getLeftThickening(j).getBoundary().area())>thickRemovalSize ){
						input.slicesThickenings[i].removeLeftThickening(j);
						j--;
					}
				}
				if(input.slicesThickenings[i].sizeLeft()==0)
					input.slicesThickenings[i].noLeftThickenings();
			}
	}
	// end of elimination
	for(int i=0; i<input.slicesThickenings.size(); i++){ //iterating through all slices
		//LEFT LUNG
		if(input.slicesThickenings[i].isLeftThickenings()){ //checking if there are thickenings on the slice
			std::cout<<"slice "<<i<<std::endl;
			for(int j=0; j<input.slicesThickenings[i].sizeLeft(); j++){//iterating through all thickenigns on the slice, left lung
				if(!input.slicesThickenings[i].getLeftThickening(j).belongTo3d())
					connectThickeningsPolygon(input, input.slicesThickenings[i].getLeftThickening(j), i, j, 1);
			}
		}
	}
	std::cout<<"---------FOUND SINGLE THICKS-------------:: "<<singlethicks<<" vs "<<tmpnum<<std::endl<<std::endl;

    emit stepProgress(1.00f);
}

void pdp::thickeningsIdentification::connectThickeningsBoundingBox(LungDataset& input, Thickening& thick, int slice, int thickNum, bool isLeft)
{
	Thickening3d temp = Thickening3d(); 
	temp.addLeftThickening(thick);
	bool foundOnSlice=false;
	for(int i=slice+1; i<input.slicesThickenings.size(); i++){ //iterating through all slices starting with the one currently processed
		foundOnSlice = false;
		//LEFT LUNG
		if(input.slicesThickenings[i].isLeftThickenings()){ //checking if there are thickenings on the slice, Left lung
			for(int j=0; j<input.slicesThickenings[i].sizeLeft(); j++){
			//CHECKING BOUNDING BOXES
				//if(input.slicesThickenings[i].getLeftThickening(j).belongTo3d()) std::cout<<"ALREADY BELONGS!!!!   ";
				if(!input.slicesThickenings[i].getLeftThickening(j).belongTo3d() && thick.areOverlappedBox(input.slicesThickenings[i].getLeftThickening(j))){ //if overlapping
					temp.addLeftThickening(input.slicesThickenings[i].getLeftThickening(j));
					input.slicesThickenings[i].getLeftThickening(j).setBelongTo3d();
					//std::cout<<"belong?"<<input.slicesThickenings[i].getLeftThickening(j).belongTo3d()<<" ";
					foundOnSlice=true;
					singlethicks++;
				}
			}
		}
		if(foundOnSlice==false)
			break;
	}
	if(temp.sizeLeft()>2){
		input.thickenings3d.push_back(temp);
		std::cout<<"3dthickening found!, size: "<<temp.sizeLeft()<<"total 3dthicks: "<<input.thickenings3d.size()<<std::endl;
	}
}

void pdp::thickeningsIdentification::connectThickeningsPolygon(LungDataset& input, Thickening& thick, int slice, int thickNum, bool isLeft)
{
	Thickening3d temp = Thickening3d(); 
	temp.addLeftThickening(thick);
	bool foundOnSlice=false;
	for(int i=slice+1; i<input.slicesThickenings.size(); i++){ //iterating through all slices starting with the one currently processed
		foundOnSlice = false;
		//LEFT LUNG
		if(input.slicesThickenings[i].isLeftThickenings()){ //checking if there are thickenings on the slice, Left lung
			for(int j=0; j<input.slicesThickenings[i].sizeLeft(); j++){
			//CHECKING BOUNDING BOXES
				//if(input.slicesThickenings[i].getLeftThickening(j).belongTo3d()) std::cout<<"ALREADY BELONGS!!!!   ";
				if(!input.slicesThickenings[i].getLeftThickening(j).belongTo3d() && thick.areOverlappedPolygon(input.slicesThickenings[i].getLeftThickening(j))){ //if overlapping
					temp.addLeftThickening(input.slicesThickenings[i].getLeftThickening(j));
					input.slicesThickenings[i].getLeftThickening(j).setBelongTo3d();
					//std::cout<<"belong?"<<input.slicesThickenings[i].getLeftThickening(j).belongTo3d()<<" ";
					foundOnSlice=true;
					singlethicks++;
				}
			}
		}
		if(foundOnSlice==false)
			break;
	}
	if(temp.sizeLeft()>2){
		input.thickenings3d.push_back(temp);
		std::cout<<"3dthickening found!, size: "<<temp.sizeLeft()<<"total 3dthicks: "<<input.thickenings3d.size()<<std::endl;
	}
}

void pdp::thickeningsIdentification::checkTissue(LungDataset&input)
{
	typedef itk::Image<double, 2>  ImageType;
	ImageType::SizeType regionSize;
	ImageType::IndexType regionIndex;
	ImageType::RegionType region;

	//preparing slices
	mitk::Image::Pointer mitkImage = input.getImage("Lungs");
	mitkImage->DisconnectPipeline();

	itk::Image<double, 3>::Pointer img;
	mitk::CastToItkImage(mitkImage, img);

	/*pdpItkSliceExtractor<double> extractor("extracting contours");
	extractor.extractSlice(img);*/

	//extractor.getSlice(i)
	//end

	double sum = 0;
	Thickening* thick;
	for(int i=0; i<input.thickenings3d.size(); i++){
		if(input.thickenings3d[i].isLeftThickenings()){
			for(int j=0; j<input.thickenings3d[i].sizeLeft(); j++){
				thick = &input.thickenings3d[i].getLeftThickening(j);
				//iterate through all points inside the thickening on the proper slice

				
				regionSize[0] = 5;
				regionSize[1] = 4;

				
				regionIndex[0] = 0;
				regionIndex[1] = 0;

				
				region.SetSize(regionSize);
				region.SetIndex(regionIndex);

				//itk::ImageRegionIterator<ImageType> imageIterator(extractor.getSlice(0),region); //watch out!!!

				//while(!imageIterator.IsAtEnd())
				//{
				//// Get the value of the current pixel
				////unsigned char val = imageIterator.Get();
				////std::cout << (int)val << std::endl;

				//// Set the current pixel to white
				//imageIterator.Set(255);

				//++imageIterator;
				//}


			}
			
		}
	}
		
}

