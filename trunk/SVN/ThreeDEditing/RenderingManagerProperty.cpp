#include "RenderingManagerProperty.h"
#include <iostream>

pdp::RenderingManagerProperty::RenderingManagerProperty()
{
	m_ReinitSurrounding = 4;
}

std::string pdp::RenderingManagerProperty::GetValueAsString() const
{
	return "A variety of pointers.";
}

bool pdp::RenderingManagerProperty::operator ==(const mitk::BaseProperty &) const
{
	return true;
}

pdp::RenderingManagerProperty::~RenderingManagerProperty()
{}