
#include <itkObjectFactoryBase.h>
#include <itkVersion.h>
#include <mitkActiveSurfaceToolFactory.h>
#include <QmitkActiveSurfaceToolGUIFactory.h>

/* No EXPORT, should not be used by any other lib, right? */
class mitkToolExtensionITKFactoryLoader : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef mitkToolExtensionITKFactoryLoader  Self;
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
      return "Generated factory for registration of MITK tools";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(mitkToolExtensionITKFactoryLoader, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      mitkToolExtensionITKFactoryLoader::Pointer factory = mitkToolExtensionITKFactoryLoader::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    mitkToolExtensionITKFactoryLoader()
    {
      // this is a dummy factory which is here just to
      // ensure that itkLoad can return something other
      // than NULL.
      // This should ensure that ITK does not unload
      // the shared library
	  itk::ObjectFactoryBase::RegisterFactory(mitk::ActiveSurfaceToolFactory::New());
	  itk::ObjectFactoryBase::RegisterFactory(QmitkActiveSurfaceToolGUIFactory::New());
    }

    ~mitkToolExtensionITKFactoryLoader()
    {
    }

  private:

    mitkToolExtensionITKFactoryLoader(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

extern "C" {

/*  itk::ObjectFactoryBase* itkLoad() */
#ifdef WIN32
__declspec(dllexport)
#endif
 itk::ObjectFactoryBase* itkLoad() 
{

  static mitkToolExtensionITKFactoryLoader::Pointer factory  = mitkToolExtensionITKFactoryLoader::New();
  return factory;
}

} // end extern "C"

