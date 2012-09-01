/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "../QmitkActiveContourToolGUI.h"

class QmitkActiveContourToolGUIFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef QmitkActiveContourToolGUIFactory  Self;
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
      return "Generated factory for QmitkActiveContourToolGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(QmitkActiveContourToolGUIFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      QmitkActiveContourToolGUIFactory::Pointer factory = QmitkActiveContourToolGUIFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    QmitkActiveContourToolGUIFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("QmitkActiveContourToolGUI",
                              "QmitkActiveContourToolGUI",
                              "Generated factory for QmitkActiveContourToolGUI",
                              1,
                              itk::CreateObjectFunction<QmitkActiveContourToolGUI>::New());
    }

    ~QmitkActiveContourToolGUIFactory()
    {
    }

  private:

    QmitkActiveContourToolGUIFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

