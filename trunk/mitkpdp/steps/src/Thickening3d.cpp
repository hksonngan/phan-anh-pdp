#include "steps/inc/thickening3d.h" 

void pdp::Thickening3d::addLeftThickening(Thickening thick){
	leftThickenings.push_back(thick);
	if(whichLungs==2)whichLungs=3;
	if(whichLungs==0)whichLungs=1;
}

void pdp::Thickening3d::addRightThickening(Thickening thick){
	rightThickenings.push_back(thick);
	if(whichLungs==1)whichLungs=3;
	if(whichLungs==0)whichLungs=2;
}

pdp::Thickening pdp::Thickening3d::getLeftThickening(int position){
	return leftThickenings[position];
}

pdp::Thickening pdp::Thickening3d::getRightThickening(int position){
	return rightThickenings[position];
}

void pdp::Thickening3d::removeLeftThickening(int position){
	leftThickenings.erase(leftThickenings.begin()+position);
}

void pdp::Thickening3d::removeRightThickening(int position){
	rightThickenings.erase(rightThickenings.begin()+position);
}

int pdp::Thickening3d::sizeLeft(){
	return leftThickenings.size();
}

int pdp::Thickening3d::sizeRight(){
	return rightThickenings.size();
}


bool pdp::Thickening3d::isLeftThickenings(void){
	if(whichLungs==1 || whichLungs==3) return true;
	else return false;
}

bool pdp::Thickening3d::isRightThickenings(void){
	if(whichLungs==2 || whichLungs==3) return true;
	else return false;
}

bool pdp::Thickening3d::isBothThickenings(void){
	if(whichLungs==3) return true;
	else return false;
}