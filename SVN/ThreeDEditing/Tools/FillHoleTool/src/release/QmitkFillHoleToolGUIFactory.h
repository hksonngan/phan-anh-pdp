/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "../QmitkFillHoleToolGUI.h"

class QmitkFillHoleToolGUIFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef QmitkFillHoleToolGUIFactory  Self;
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
      return "Generated factory for QmitkFillHoleToolGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(QmitkFillHoleToolGUIFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      QmitkFillHoleToolGUIFactory::Pointer factory = QmitkFillHoleToolGUIFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    QmitkFillHoleToolGUIFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("QmitkFillHoleToolGUI",
                              "QmitkFillHoleToolGUI",
                              "Generated factory for QmitkFillHoleToolGUI",
                              1,
                              itk::CreateObjectFunction<QmitkFillHoleToolGUI>::New());
    }

    ~QmitkFillHoleToolGUIFactory()
    {
    }

  private:

    QmitkFillHoleToolGUIFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

