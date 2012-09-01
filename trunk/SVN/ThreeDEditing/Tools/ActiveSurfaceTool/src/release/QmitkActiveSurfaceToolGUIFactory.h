/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "../QmitkActiveSurfaceToolGUI.h"

class QmitkActiveSurfaceToolGUIFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef QmitkActiveSurfaceToolGUIFactory  Self;
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
      return "Generated factory for QmitkActiveSurfaceToolGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(QmitkActiveSurfaceToolGUIFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      QmitkActiveSurfaceToolGUIFactory::Pointer factory = QmitkActiveSurfaceToolGUIFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    QmitkActiveSurfaceToolGUIFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("QmitkActiveSurfaceToolGUI",
                              "QmitkActiveSurfaceToolGUI",
                              "Generated factory for QmitkActiveSurfaceToolGUI",
                              1,
                              itk::CreateObjectFunction<QmitkActiveSurfaceToolGUI>::New());
    }

    ~QmitkActiveSurfaceToolGUIFactory()
    {
    }

  private:

    QmitkActiveSurfaceToolGUIFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

