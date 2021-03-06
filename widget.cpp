#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("QtRelease");
    setMinimumWidth(800);
    setMinimumHeight(600);
    initControls();
    copytPool = new QThreadPool(this);
    copytPool->setMaxThreadCount(5);
}

Widget::~Widget()
{
    delete ui;
}
void Widget::qtcboxToggled(bool toggled)
{
    if(toggled)
    {
        for(int i = 0; i < qtlibviewmodel->rowCount(); i++)
            qtlibviewmodel->item(i)->setCheckState(Qt::Checked);

    }
    else
    {
        for(int i = 0; i < qtlibviewmodel->rowCount(); i++)
            qtlibviewmodel->item(i)->setCheckState(Qt::Unchecked);
    }
}
void Widget::analyzeClicked()
{
    infol->setText("");
    exenameEdit->setEnabled(false);
    if(exenameEdit->text() == "")
    {
        QMessageBox::information(this, "Tip", "Enter \"xxx.exe\" to lineedit, and run it(no full path)");
        exenameEdit->setEnabled(true);
        return;
    }
    qtlibrarymap.clear();
    getPeDependDllInfo(exenameEdit->text());
    if(qtlibrarymap.isEmpty())
    {
        QMessageBox::information(this, "Tip", "Enter \"xxx.exe\" to lineedit, and run it(no full path)");
        exenameEdit->setEnabled(true);
        return;
    }
    initLibView();
    infol->setText("Geted");
    exenameEdit->setDisabled(false);
}
void Widget::getNeedLibList(void)
{
    libneed.clear();
    QString tmp;
    for(int i = 0; i < qtlibviewmodel->rowCount(); i++)
    {
        if(qtlibviewmodel->item(i, 0)->checkState() == Qt::Checked)
            libneed.append(qtlibviewmodel->index(i,0).data().toString());

    }
}
void  Widget::createNeededDir(void)
{
    infol->setText("start create some dir for need");
    for(int i =0; i < libneeddir.count(); i++)
    {
        QDir dir(libneeddir.at(i));
        if(dir.exists())
          continue;
        else
           dir.mkdir(libneeddir.at(i));
    }
    infol->setText("created dir");
}
QString Widget::getCopyName(const QString& sourcename)
{
    QString ret = qtlibrarymap[sourcename];
    if(ret == "")
        ret = winlibrarymap[sourcename];
    if(ret == "")
        ret = thirdlibrarymap[sourcename];
    return ret;
}
void Widget::startCopyClicked()
{
    if(qtlibrarymap.isEmpty())
    {
        QMessageBox::information(this, "Tip", "Enter \"xxx.exe\" to lineedit, and run it(no full path)");
        exenameEdit->setEnabled(true);
        return;
    }
    createNeededDir();
    getNeedLibList();
    infol->setText("start copy");
    QString tmp;
    for(int i = 0; i < libneed.count(); i++)
    {
        tmp = libneed.at(i);
        copytPool->start(new CopyTask(tmp, getCopyName(tmp)));
    }
    copytPool->waitForDone();
    infol->setText("copy success");
    exenameEdit->setText("Give me a star at https://github.com/chfyjy/QtWindeploy.git");
}
void Widget::initLibView(void)
{
    QMap<QString, QString>::iterator it;
    qtlibviewmodel->clear();
    for(it = qtlibrarymap.begin(); it != qtlibrarymap.end(); it++)
    {
        QStandardItem* item = new QStandardItem(it.key());
        item->setCheckable( true );
        item->setCheckState(Qt::Checked);
        qtlibviewmodel->appendRow(item);
    }
    for(it = thirdlibrarymap.begin(); it != thirdlibrarymap.end(); it++)
    {
        QStandardItem* item = new QStandardItem(it.key());
        item->setCheckable( true );
        qtlibviewmodel->appendRow(item);
    }
    for(it = winlibrarymap.begin(); it != winlibrarymap.end(); it++)
    {
        QStandardItem* item = new QStandardItem(it.key());
        item->setCheckable( true );
        qtlibviewmodel->appendRow(item);
    }
}
void Widget::initControls()
{
    QLabel *exenamel = new QLabel(this);
    exenamel->setText("exe name");
    exenameEdit = new QLineEdit(this);
    analyze = new QPushButton(this);
    analyze->setText("get dll info");
    connect(analyze, SIGNAL(clicked()), this, SLOT(analyzeClicked()));
    startCopy = new QPushButton(this);
    startCopy->setMaximumWidth(40);
    startCopy->setText("copy");
    connect(startCopy, SIGNAL(clicked()), this, SLOT(startCopyClicked()));
    QHBoxLayout *exeNamelayout = new QHBoxLayout();
    exeNamelayout->addWidget(exenamel);
    exeNamelayout->addWidget(exenameEdit);
    exeNamelayout->addWidget(analyze);
    exeNamelayout->addWidget(startCopy);

    qtcbox = new QCheckBox(this);
    qtcbox->setText("select all");
    qtcbox->setMaximumWidth(150);
    qtcbox->setMinimumWidth(150);
    connect(qtcbox, SIGNAL(toggled(bool)), this, SLOT(qtcboxToggled(bool)));
    QHBoxLayout *cboxLayout = new QHBoxLayout();
    infol = new QLabel(this);
    infol->setText("");
    cboxLayout->addWidget(qtcbox);
    cboxLayout->addWidget(infol);

    qtlibview = new QListView(this);
    qtlibviewmodel = new QStandardItemModel(this);
    qtlibview->setModel(qtlibviewmodel);
    qtlibview->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QVBoxLayout *viewLayout = new QVBoxLayout();
    viewLayout->addWidget(qtlibview);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(exeNamelayout);
    layout->addLayout(cboxLayout);
    layout->addLayout(viewLayout);
    setLayout(layout);
    setGeometry(100,100,400, 300);
}
inline QString GetEXEDir(QString EXEAbsolatePath, const QString& EXEName)
{
    QString ret = EXEAbsolatePath.remove(EXEName);
    return  ret;
}
QString GetDLLName(const QString& DLLAbsolatePath, const QString& re)
{
    QStringList tmp = DLLAbsolatePath.split(re);
    return  tmp.at(tmp.count()-1);
}
QString GetPluginsDirName(const QString& DLLAbsolatePath)
{
    QStringList tmp = DLLAbsolatePath.split("/plugins/");
    QString tmpstr = tmp.at(tmp.count()-1);
    tmp = tmpstr.split("/");
    tmpstr = tmp.at(0);
    return  tmpstr;
}
void Widget::getPeDependDllInfo(const QString& aimexeName)
{
    PROCESSENTRY32 pe32;//进程结构
    pe32.dwSize = sizeof(pe32); //在使用这个结构前,先设置它的大小
    //给系统内所有的进程拍个快照
    HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    //某个进程所有的DLL快照句柄
    HANDLE hModuleSnap = NULL;
    if(hProcessSnap == INVALID_HANDLE_VALUE)
    {
        printf("CreateTollHelp32Snapshot Error!!\n");
        return ;
    }
    BOOL bMore = ::Process32First(hProcessSnap, &pe32);
    HANDLE hProcess = 0;
    WCHAR procPath[_MAX_PATH]={0};
    MODULEENTRY32 lpme;  //DLL结构
    lpme.dwSize = sizeof(MODULEENTRY32);//在使用这个结构前,先设置它的大小
    BOOL bRet = FALSE;
    QString exeName, exePath, dllPath,tmpdllPath, exeDir;
    //遍历进程快照,显示每个进程的信息
    while(bMore)
    {
        //打开一个已存在的进程对象,并返回进程的句柄
        hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pe32.th32ProcessID);
        //得到该进程的全路径
        GetModuleFileNameEx(hProcess,NULL,procPath, _MAX_PATH);
        if(QString::fromStdWString(  pe32.szExeFile) == aimexeName)
        {
            exeName = QString::fromStdWString(pe32.szExeFile);
            exeName.replace('\\','/');
            exePath = QString::fromStdWString(procPath).replace('\\','/');
            exePath.replace('\\','/');
            exeDir = GetEXEDir(exePath, exeName);
            //qDebug() << exeDir;
            //给一个已存在的进程内所有的DLL拍个快照
            hModuleSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);
            bRet = ::Module32First(hModuleSnap, &lpme);
            if(bRet)
                bRet = ::Module32Next(hModuleSnap, &lpme);
            else
                return;
            //遍历DLL快照,显示该进程所加在的DLL信息
            while(bRet)
            {
                dllPath = QString::fromStdWString( lpme.szExePath);

                dllPath.replace('\\','/');
                //qDebug() << dllPath;
                tmpdllPath = dllPath.toLower();
                if(tmpdllPath.contains("qt"))
                {
                    if(tmpdllPath.contains("plugins"))
                    {
                        qtlibrarymap.insert(dllPath, exeDir + GetDLLName(dllPath, "/plugins/"));
                        libneeddir.append(exeDir +GetPluginsDirName(dllPath));
                    }
                    else
                        qtlibrarymap.insert(dllPath, exeDir + GetDLLName(dllPath, "/"));
                }
                else if(tmpdllPath.contains("windows"))
                {
                    //qDebug() << dllPath << exeDir + GetDLLName(dllPath, "/");
                    winlibrarymap.insert(dllPath, exeDir + GetDLLName(dllPath, "/"));
                }
                else
                {
                    //qDebug() << dllPath << exeDir + GetDLLName(dllPath, "/");
                    thirdlibrarymap.insert(dllPath, exeDir + GetDLLName(dllPath, "/"));
                }
                bRet = ::Module32Next(hModuleSnap, &lpme);
            }
            //关闭snapshot对象
            ::CloseHandle(hModuleSnap);
        }
        bMore = ::Process32Next(hProcessSnap, &pe32);
    }
    //关闭snapshot对象
    ::CloseHandle(hProcessSnap);
}
