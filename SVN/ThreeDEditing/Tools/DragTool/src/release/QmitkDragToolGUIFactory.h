/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "../QmitkDragToolGUI.h"

class QmitkDragToolGUIFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef QmitkDragToolGUIFactory  Self;
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
      return "Generated factory for QmitkDragToolGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(QmitkDragToolGUIFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      QmitkDragToolGUIFactory::Pointer factory = QmitkDragToolGUIFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    QmitkDragToolGUIFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("QmitkDragToolGUI",
                              "QmitkDragToolGUI",
                              "Generated factory for QmitkDragToolGUI",
                              1,
                              itk::CreateObjectFunction<QmitkDragToolGUI>::New());
    }

    ~QmitkDragToolGUIFactory()
    {
    }

  private:

    QmitkDragToolGUIFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};
