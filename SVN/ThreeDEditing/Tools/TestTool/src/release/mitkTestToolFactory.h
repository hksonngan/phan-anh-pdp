/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances.
 */

#include <itkObjectFactoryBase.h>

#include "../mitkTestTool.h"

namespace mitk {

/* class  AddThreeDTool.cppFactory : public ::itk::ObjectFactoryBase */
class TestToolFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef TestToolFactory  Self;
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
      return "Generated factory for TestTool.cpp";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(TestToolFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      TestToolFactory::Pointer factory = TestToolFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    TestToolFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("mitkTool",
                              "mitkTestTool",
                              "Generated factory for TestTool",
                              1,
                              itk::CreateObjectFunction<TestTool>::New());
    }

    ~TestToolFactory()
    {
    }

  private:

    TestToolFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

} // end namespace

