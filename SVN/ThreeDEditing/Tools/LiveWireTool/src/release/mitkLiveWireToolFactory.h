/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances.
 */

#include <itkObjectFactoryBase.h>

#include "../mitkLiveWireTool.h"

namespace mitk {

/* class  AddThreeDTool.cppFactory : public ::itk::ObjectFactoryBase */
class LiveWireToolFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef LiveWireToolFactory  Self;
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
      return "Generated factory for LiveWireTool.cpp";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(LiveWireToolFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      LiveWireToolFactory::Pointer factory = LiveWireToolFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    LiveWireToolFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("mitkTool",
                              "mitkLiveWireTool",
                              "Generated factory for LiveWireTool",
                              1,
                              itk::CreateObjectFunction<LiveWireTool>::New());
    }

    ~LiveWireToolFactory()
    {
    }

  private:

    LiveWireToolFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

} // end namespace

