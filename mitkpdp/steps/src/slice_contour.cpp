#include "steps/inc/slice_contour.h" 

typedef itk::PolyLineParametricPath< 2 >::Pointer PathPointer;

void pdp::sliceContour::setLeftLung(PathPointer& path){
	leftLung=path;
	if(whichLungs==2)whichLungs=3;
	if(whichLungs==0)whichLungs=1;
}

void pdp::sliceContour::setRightLung(PathPointer& path){
	rightLung=path;
	if(whichLungs==1)whichLungs=3;
	if(whichLungs==0)whichLungs=2;
}

PathPointer pdp::sliceContour::getLeftLung(void){
	return leftLung;
}

PathPointer pdp::sliceContour::getRightLung(void){
	return rightLung;
}

void pdp::sliceContour::setLeftHull(PathPointer& path){
	leftHull=path;
}

void pdp::sliceContour::setRightHull(PathPointer& path){
	rightHull=path;
}

PathPointer pdp::sliceContour::getLeftHull(void){
	return leftHull;
}

PathPointer pdp::sliceContour::getRightHull(void){
	return rightHull;
}

bool pdp::sliceContour::isLeftLung(void){
	if(whichLungs==1 || whichLungs==3) return true;
	else return false;
}

bool pdp::sliceContour::isRightLung(void){
	if(whichLungs==2 || whichLungs==3) return true;
	else return false;
}

bool pdp::sliceContour::isBothLungs(void){
	if(whichLungs==3) return true;
	else return false;
}