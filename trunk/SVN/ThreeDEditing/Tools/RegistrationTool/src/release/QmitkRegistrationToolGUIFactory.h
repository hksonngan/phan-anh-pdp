/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "../QmitkRegistrationToolGUI.h"

class QmitkRegistrationToolGUIFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef QmitkRegistrationToolGUIFactory  Self;
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
      return "Generated factory for QmitkRegistrationToolGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(QmitkRegistrationToolGUIFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      QmitkRegistrationToolGUIFactory::Pointer factory = QmitkRegistrationToolGUIFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    QmitkRegistrationToolGUIFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("QmitkRegistrationToolGUI",
                              "QmitkRegistrationToolGUI",
                              "Generated factory for QmitkRegistrationToolGUI",
                              1,
                              itk::CreateObjectFunction<QmitkRegistrationToolGUI>::New());
    }

    ~QmitkRegistrationToolGUIFactory()
    {
    }

  private:

    QmitkRegistrationToolGUIFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

