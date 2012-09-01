/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "QmitkComplexExternalToolGUI.h"

//class @EXPORT_MACRO@ ComplexExternalToolGUIExtensionITKFactory : public ::itk::ObjectFactoryBase
class ComplexExternalToolGUIExtensionITKFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef ComplexExternalToolGUIExtensionITKFactory  Self;
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
      return "Hallo\n";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(ComplexExternalToolGUIExtensionITKFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      ComplexExternalToolGUIExtensionITKFactory::Pointer factory = ComplexExternalToolGUIExtensionITKFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    ComplexExternalToolGUIExtensionITKFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("QmitkComplexExternalToolGUI",
                              "QmitkComplexExternalToolGUI",
                              "Hallo",
                              1,
                              itk::CreateObjectFunction<QmitkComplexExternalToolGUI>::New());
    }

    ~ComplexExternalToolGUIExtensionITKFactory()
    {
    }

  private:

    ComplexExternalToolGUIExtensionITKFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

