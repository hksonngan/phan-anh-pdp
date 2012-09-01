#include "steps/inc/slice_thickenings.h" 

void pdp::sliceThickenings::noLeftThickenings(){
	if(whichLungs==3)whichLungs=2;
	else if(whichLungs==1)whichLungs=0;
}
void pdp::sliceThickenings::noRightThickenings(){
	if(whichLungs==3)whichLungs=1;
	else if(whichLungs==2)whichLungs=0;
}
void pdp::sliceThickenings::addLeftThickening(Thickening thick){
	leftThickenings.push_back(thick);
	if(whichLungs==2)whichLungs=3;
	else if(whichLungs==0)whichLungs=1;
}

void pdp::sliceThickenings::addLeftThickening(std::vector<Thickening> thicks){
	for(int i=0; i<thicks.size(); i++)
		leftThickenings.push_back(thicks[i]);
	if(whichLungs==2)whichLungs=3;
	else if(whichLungs==0)whichLungs=1;
}

void pdp::sliceThickenings::addRightThickening(Thickening thick){
	rightThickenings.push_back(thick);
	if(whichLungs==1)whichLungs=3;
	else if(whichLungs==0)whichLungs=2;
}

void pdp::sliceThickenings::addRightThickening(std::vector<Thickening> thicks){
	for(int i=0; i<thicks.size(); i++)
		rightThickenings.push_back(thicks[i]);
	if(whichLungs==1)whichLungs=3;
	else if(whichLungs==0)whichLungs=2;
}


pdp::Thickening& pdp::sliceThickenings::getLeftThickening(int position){
	return leftThickenings[position];
}

pdp::Thickening& pdp::sliceThickenings::getRightThickening(int position){
	return rightThickenings[position];
}

void pdp::sliceThickenings::removeLeftThickening(int position){
	leftThickenings.erase(leftThickenings.begin()+position);
}

void pdp::sliceThickenings::removeRightThickening(int position){
	rightThickenings.erase(rightThickenings.begin()+position);
}

int pdp::sliceThickenings::sizeLeft(){
	return leftThickenings.size();
}

int pdp::sliceThickenings::sizeRight(){
	return rightThickenings.size();
}


bool pdp::sliceThickenings::isLeftThickenings(void){
	if(whichLungs==1 || whichLungs==3) return true;
	else return false;
}

bool pdp::sliceThickenings::isRightThickenings(void){
	if(whichLungs==2 || whichLungs==3) return true;
	else return false;
}

bool pdp::sliceThickenings::isBothThickenings(void){
	if(whichLungs==3) return true;
	else return false;
}
