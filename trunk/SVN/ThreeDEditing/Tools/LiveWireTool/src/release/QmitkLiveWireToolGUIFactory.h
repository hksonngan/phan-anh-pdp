/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "../QmitkLiveWireToolGUI.h"

class QmitkLiveWireToolGUIFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef QmitkLiveWireToolGUIFactory  Self;
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
      return "Generated factory for QmitkLiveWireToolGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(QmitkLiveWireToolGUIFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      QmitkLiveWireToolGUIFactory::Pointer factory = QmitkLiveWireToolGUIFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    QmitkLiveWireToolGUIFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("QmitkLiveWireToolGUI",
                              "QmitkLiveWireToolGUI",
                              "Generated factory for QmitkLiveWireToolGUI",
                              1,
                              itk::CreateObjectFunction<QmitkLiveWireToolGUI>::New());
    }

    ~QmitkLiveWireToolGUIFactory()
    {
    }

  private:

    QmitkLiveWireToolGUIFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

