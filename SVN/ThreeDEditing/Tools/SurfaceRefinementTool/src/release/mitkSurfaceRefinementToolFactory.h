/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances.
 */

#include <itkObjectFactoryBase.h>

#include "../mitkSurfaceRefinementTool.h"

namespace mitk {

/* class  AddThreeDTool.cppFactory : public ::itk::ObjectFactoryBase */
class SurfaceRefinementToolFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef SurfaceRefinementToolFactory  Self;
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
      return "Generated factory for SurfaceRefinementTool.cpp";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(SurfaceRefinementToolFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      SurfaceRefinementToolFactory::Pointer factory = SurfaceRefinementToolFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    SurfaceRefinementToolFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("mitkTool",
                              "mitkSurfaceRefinementTool",
                              "Generated factory for SurfaceRefinementTool",
                              1,
                              itk::CreateObjectFunction<SurfaceRefinementTool>::New());
    }

    ~SurfaceRefinementToolFactory()
    {
    }

  private:

    SurfaceRefinementToolFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

} // end namespace

