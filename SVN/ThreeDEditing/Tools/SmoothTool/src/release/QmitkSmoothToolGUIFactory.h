/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "../QmitkSmoothToolGUI.h"

class QmitkSmoothToolGUIFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef QmitkSmoothToolGUIFactory  Self;
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
      return "Generated factory for QmitkSmoothToolGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(QmitkSmoothToolGUIFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      QmitkSmoothToolGUIFactory::Pointer factory = QmitkSmoothToolGUIFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    QmitkSmoothToolGUIFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("QmitkSmoothToolGUI",
                              "QmitkSmoothToolGUI",
                              "Generated factory for QmitkSmoothToolGUI",
                              1,
                              itk::CreateObjectFunction<QmitkSmoothToolGUI>::New());
    }

    ~QmitkSmoothToolGUIFactory()
    {
    }

  private:

    QmitkSmoothToolGUIFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

