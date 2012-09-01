/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "Qmitk../QmitkSurfaceRefinementToolGUI.cppGUI.h"

//class  ../QmitkSurfaceRefinementToolGUIFactory : public ::itk::ObjectFactoryBase
class ../QmitkSurfaceRefinementToolGUIFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef ../QmitkSurfaceRefinementToolGUIFactory  Self;
    typedef itk::ObjectFactoryBase  Superclass;
    typedef itk::SmartPointer<Self>  Pointer;
    typedef itk::SmartPointer<const Self>  ConstPointer;
     
    /* Methods from ObjectFactoryBase */
    virtual const char* GetITKSourceVersion() const
    {
      return ITK_SOURCE_VERSION;
    }\
    
    virtual const char* GetDescription() const
    {
      return "Generated factory for Qmitk../QmitkSurfaceRefinementToolGUI.cppGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(../QmitkSurfaceRefinementToolGUIFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      ../QmitkSurfaceRefinementToolGUIFactory::Pointer factory = ../QmitkSurfaceRefinementToolGUIFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    ../QmitkSurfaceRefinementToolGUIFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("Qmitk../QmitkSurfaceRefinementToolGUI.cppGUI",
                              "Qmitk../QmitkSurfaceRefinementToolGUI.cppGUI",
                              "Generated factory for Qmitk../QmitkSurfaceRefinementToolGUI.cppGUI",
                              1,
                              itk::CreateObjectFunction<Qmitk../QmitkSurfaceRefinementToolGUI.cppGUI>::New());
    }

    ~../QmitkSurfaceRefinementToolGUIFactory()
    {
    }

  private:

    ../QmitkSurfaceRefinementToolGUIFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

