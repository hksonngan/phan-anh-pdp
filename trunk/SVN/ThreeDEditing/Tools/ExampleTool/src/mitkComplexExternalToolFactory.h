/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "mitkComplexExternalTool.h"

namespace mitk {

/* class  ComplexExternalToolFactory : public ::itk::ObjectFactoryBase */
class ComplexExternalToolFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef ComplexExternalToolFactory  Self;
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
      return "Generated factory for ComplexExternalTool";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(ComplexExternalToolFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      ComplexExternalToolFactory::Pointer factory = ComplexExternalToolFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    ComplexExternalToolFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("mitkTool",
                              "ComplexExternalTool",
                              "Generated factory for ComplexExternalTool",
                              1,
                              itk::CreateObjectFunction<ComplexExternalTool>::New());
    }

    ~ComplexExternalToolFactory()
    {
    }

  private:

    ComplexExternalToolFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

} // end namespace

