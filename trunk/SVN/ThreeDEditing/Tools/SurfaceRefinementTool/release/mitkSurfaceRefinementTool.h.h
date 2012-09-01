/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "Qmitk../mitkSurfaceRefinementTool.hGUI.h"

//class  ../mitkSurfaceRefinementTool.h : public ::itk::ObjectFactoryBase
class ../mitkSurfaceRefinementTool.h : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef ../mitkSurfaceRefinementTool.h  Self;
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
      return "Generated factory for Qmitk../mitkSurfaceRefinementTool.hGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(../mitkSurfaceRefinementTool.h, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      ../mitkSurfaceRefinementTool.h::Pointer factory = ../mitkSurfaceRefinementTool.h::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    ../mitkSurfaceRefinementTool.h()
    {
      itk::ObjectFactoryBase::RegisterOverride("Qmitk../mitkSurfaceRefinementTool.hGUI",
                              "Qmitk../mitkSurfaceRefinementTool.hGUI",
                              "Generated factory for Qmitk../mitkSurfaceRefinementTool.hGUI",
                              1,
                              itk::CreateObjectFunction<Qmitk../mitkSurfaceRefinementTool.hGUI>::New());
    }

    ~../mitkSurfaceRefinementTool.h()
    {
    }

  private:

    ../mitkSurfaceRefinementTool.h(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

