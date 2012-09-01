/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "QmitkAddThreeDToolGUI.h"

//class  ADDThreeDToolGUIFactory : public ::itk::ObjectFactoryBase
class AddThreeDToolGUIFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef AddThreeDToolGUIFactory  Self;
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
      return "Generated factory for QmitkADDThreeDToolGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(AddThreeDToolGUIFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      AddThreeDToolGUIFactory::Pointer factory = AddThreeDToolGUIFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    AddThreeDToolGUIFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("QmitkAddThreeDToolGUI",
                              "QmitkAddThreeDToolGUI",
                              "Generated factory for QmitkAddThreeDToolGUI",
                              1,
                              itk::CreateObjectFunction<AddThreeDToolGUI>::New());
    }

    ~AddThreeDToolGUIFactory()
    {
    }

  private:

    AddThreeDToolGUIFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

