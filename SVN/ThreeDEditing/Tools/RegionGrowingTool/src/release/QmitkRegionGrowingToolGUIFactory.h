/**
 * This defines an itk::ObjectFactory subclass specific for
 * creation of mitk::Tool instances (in this case mitk::PlainExternalTool).
 */

#include <itkObjectFactoryBase.h>

#include "../QmitkRegionGrowingToolGUI.h"

class QmitkRegionGrowingToolGUIFactory : public ::itk::ObjectFactoryBase
{
  public:  

    /* ITK typedefs */
    typedef QmitkRegionGrowingToolGUIFactory  Self;
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
      return "Generated factory for QmitkRegionGrowingToolGUI";
    }\
    
    /* Method for class instantiation. */
    itkFactorylessNewMacro(Self);
      
    /* Run-time type information (and related methods). */
    itkTypeMacro(QmitkRegionGrowingToolGUIFactory, itkObjectFactoryBase);

    /* Register one factory of this type  */
    static void RegisterOneFactory()
    {
      QmitkRegionGrowingToolGUIFactory::Pointer factory = QmitkRegionGrowingToolGUIFactory::New();
      itk::ObjectFactoryBase::RegisterFactory(factory);
    }
    
  protected:

    QmitkRegionGrowingToolGUIFactory()
    {
      itk::ObjectFactoryBase::RegisterOverride("QmitkRegionGrowingToolGUI",
                              "QmitkRegionGrowingToolGUI",
                              "Generated factory for QmitkRegionGrowingToolGUI",
                              1,
                              itk::CreateObjectFunction<QmitkRegionGrowingToolGUI>::New());
    }

    ~QmitkRegionGrowingToolGUIFactory()
    {
    }

  private:

    QmitkRegionGrowingToolGUIFactory(const Self&);    /* purposely not implemented */
    void operator=(const Self&);    /* purposely not implemented */

};

