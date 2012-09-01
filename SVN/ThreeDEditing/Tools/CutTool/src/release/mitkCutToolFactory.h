/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances.
 */

#include <itkObjectFactoryBase.h>

#include "../mitkCutTool.h"

namespace mitk {

/* class  AddThreeDTool.cppFactory : public ::itk::ObjectFactoryBase */
class CutToolFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef CutToolFactory  Self;
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
      return "Generated factory for CutTool.cpp";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(CutToolFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      CutToolFactory::Pointer factory = CutToolFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    CutToolFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("mitkTool",
                              "mitkCutTool",
                              "Generated factory for CutTool",
                              1,
                              itk::CreateObjectFunction<CutTool>::New());
    }

    ~CutToolFactory()
    {
    }

  private:

    CutToolFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

} // end namespace

