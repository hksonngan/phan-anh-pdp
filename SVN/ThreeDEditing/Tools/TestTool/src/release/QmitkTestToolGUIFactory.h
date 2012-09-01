/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "../QmitkTestToolGUI.h"

class QmitkTestToolGUIFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef QmitkTestToolGUIFactory  Self;
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
      return "Generated factory for QmitkTestToolGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(QmitkTestToolGUIFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      QmitkTestToolGUIFactory::Pointer factory = QmitkTestToolGUIFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    QmitkTestToolGUIFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("QmitkTestToolGUI",
                              "QmitkTestToolGUI",
                              "Generated factory for QmitkTestToolGUI",
                              1,
                              itk::CreateObjectFunction<QmitkTestToolGUI>::New());
    }

    ~QmitkTestToolGUIFactory()
    {
    }

  private:

    QmitkTestToolGUIFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

