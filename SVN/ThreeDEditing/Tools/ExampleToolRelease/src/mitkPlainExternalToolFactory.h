/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "mitkPlainExternalTool.h"

namespace mitk {

class PlainExternalToolFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef PlainExternalToolFactory  Self;
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
      return "Generated factory for PlainExternalTool";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(PlainExternalToolFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      PlainExternalToolFactory::Pointer factory = PlainExternalToolFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    PlainExternalToolFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("mitkTool",
                              "PlainExternalTool",
                              "Generated factory for PlainExternalTool",
                              1,
                              itk::CreateObjectFunction<PlainExternalTool>::New());
    }

    ~PlainExternalToolFactory()
    {
    }

  private:

    PlainExternalToolFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

} // end namespace

