/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances.
 */

#include <itkObjectFactoryBase.h>

#include "../mitkRegistrationTool.h"

namespace mitk {

/* class  AddThreeDTool.cppFactory : public ::itk::ObjectFactoryBase */
class RegistrationToolFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef RegistrationToolFactory  Self;
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
      return "Generated factory for RegistrationTool.cpp";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(RegistrationToolFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      RegistrationToolFactory::Pointer factory = RegistrationToolFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    RegistrationToolFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("mitkTool",
                              "mitkRegistrationTool",
                              "Generated factory for RegistrationTool",
                              1,
                              itk::CreateObjectFunction<RegistrationTool>::New());
    }

    ~RegistrationToolFactory()
    {
    }

  private:

    RegistrationToolFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

} // end namespace

