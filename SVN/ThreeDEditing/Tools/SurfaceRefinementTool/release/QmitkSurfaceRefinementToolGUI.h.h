/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "Qmitk../QmitkSurfaceRefinementToolGUI.hGUI.h"

//class  ../QmitkSurfaceRefinementToolGUI.h : public ::itk::ObjectFactoryBase
class ../QmitkSurfaceRefinementToolGUI.h : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef ../QmitkSurfaceRefinementToolGUI.h  Self;
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
      return "Generated factory for Qmitk../QmitkSurfaceRefinementToolGUI.hGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(../QmitkSurfaceRefinementToolGUI.h, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      ../QmitkSurfaceRefinementToolGUI.h::Pointer factory = ../QmitkSurfaceRefinementToolGUI.h::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    ../QmitkSurfaceRefinementToolGUI.h()
    {
      itk::ObjectFactoryBase::RegisterOverride("Qmitk../QmitkSurfaceRefinementToolGUI.hGUI",
                              "Qmitk../QmitkSurfaceRefinementToolGUI.hGUI",
                              "Generated factory for Qmitk../QmitkSurfaceRefinementToolGUI.hGUI",
                              1,
                              itk::CreateObjectFunction<Qmitk../QmitkSurfaceRefinementToolGUI.hGUI>::New());
    }

    ~../QmitkSurfaceRefinementToolGUI.h()
    {
    }

  private:

    ../QmitkSurfaceRefinementToolGUI.h(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

