#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QtGui>

#include <QtWidgets>



//#include "gdal_version.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "gdal_utils_priv.h"

#include "info_H.h"

bool RetiraParenteses(char *Dest,char *Str) {

    unsigned int i,j;
    j = 0;
    for (i = 0 ; i < strlen(Str) ; i++) {
        if ((Str[i] != '(') && (Str[i] != ')')) {
            Dest[j] = Str[i];
            j++;
        }
    }
    Dest[j] = 0;

    return true;
}

bool DecodeXMP(QStringList &stringList, GDALDataset *poSrcDS) {
    QString S2;
    char** papszIter;
    char** papszMetadataDomain = CSLDuplicate(poSrcDS->GetMetadataDomainList());

    stringList.clear();

    papszIter = papszMetadataDomain;
    while(papszIter && *papszIter)
    {
        S2 = *papszIter;

        if (S2.length()) {
            stringList.append(S2);
//            ui->txt_Output->append(S2);
        }
        papszIter++;
    }


    bool Ok = false;
    for (int i = 0 ; i < stringList.length() ; i ++) {
        if (stringList.at(i) == "xml:XMP") Ok = true;
    }

    if (!Ok) return false;

    char** papszMetadataXMP = CSLDuplicate(poSrcDS->GetMetadata("xml:XMP"));
    QString itemText = QString::fromUtf8(*papszMetadataXMP);

    stringList.clear();
    QTextStream textStream(&itemText);
    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else
//            if (line.contains("dji:")) stringList.append(line);
            stringList.append(line);
    }
    return true;
}


double getLATfromEXIF(GDALDataset *poSrcDS) {
    char    InputStr[100],Temp[100];
    int     i,k;
    double  Val;

    i = 0;
    k = 0;
    strcpy(Temp,"");
    strcpy(InputStr,poSrcDS->GetMetadataItem("EXIF_GPSLatitude"));

    do {
        if(isdigit(InputStr[i])) {
            Temp[k] = InputStr[i];
            k++;
        }
        i++;
    } while (InputStr[i] != ')');
    Temp[k] = 0;
    Val = atof(Temp);

    k = 0;
    do {
        if(isdigit(InputStr[i])) {
            Temp[k] = InputStr[i];
            k++;
        }
        i++;
    } while (InputStr[i] != ')');
    Temp[k] = 0;
    Val = Val + (atof(Temp) / 60.0);

    k = 0;
    do {
        if((isdigit(InputStr[i])) || (InputStr[i] == '.')) {
            Temp[k] = InputStr[i];
            k++;
        }
        i++;
    } while (InputStr[i] != ')');
    Temp[k] = 0;
    Val = Val + (atof(Temp) / 3600.0);

    strcpy(InputStr,poSrcDS->GetMetadataItem("EXIF_GPSLatitudeRef"));

    if (strcmp(InputStr,"S") == 0) Val = -Val;
    return(Val);
}

double getLONfromEXIF(GDALDataset *poSrcDS) {
    char    InputStr[100],Temp[100];
    int     i,k;
    double  Val;

    i = 0;
    k = 0;
    strcpy(Temp,"");
    strcpy(InputStr,poSrcDS->GetMetadataItem("EXIF_GPSLongitude"));

    do {
        if(isdigit(InputStr[i])) {
            Temp[k] = InputStr[i];
            k++;
        }
        i++;
    } while (InputStr[i] != ')');
    Temp[k] = 0;
    Val = atof(Temp);

    k = 0;
    do {
        if(isdigit(InputStr[i])) {
            Temp[k] = InputStr[i];
            k++;
        }
        i++;
    } while (InputStr[i] != ')');
    Temp[k] = 0;
    Val = Val + (atof(Temp) / 60.0);

    k = 0;
    do {
        if((isdigit(InputStr[i])) || (InputStr[i] == '.')) {
            Temp[k] = InputStr[i];
            k++;
        }
        i++;
    } while (InputStr[i] != ')');
    Temp[k] = 0;
    Val = Val + (atof(Temp) / 3600.0);

    strcpy(InputStr,poSrcDS->GetMetadataItem("EXIF_GPSLongitudeRef"));

    if (strcmp(InputStr,"W") == 0) Val = -Val;
    return(Val);
}

double GetXMPItem(QStringList Lista,QString Item) {

    int i,j,k;
    char strVal[20],Temp[100];
    strcpy(strVal,"0");
    for (i = 0 ; i < Lista.length() ; i++) {
        if (Lista.at(i).contains(Item)) {
            strcpy(Temp,Lista.at(i).toStdString().c_str());
            for (j = 0 ; j < strlen(Temp) ; j++) {
                if (Temp[j] == ':') break;
            }
            k = 0;
            for ( ; j < strlen(Temp) ; j++) {
                if ((isdigit(Temp[j])) || (Temp[j] == '.') || (Temp[j] == '-')) {
                    strVal[k] = Temp[j];
                    k++;
                }
            }
            strVal[k] = 0;
        }
    }
    return(atof(strVal));
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QStringList     stringList;
    double          Lat,Lon,GPS_Alt,Relative_Alt;
    char            Lat_Str[50],Lon_Str[50],GPS_Alt_Str[50],Temp[50];
    QString         Name,Name_Saida,Out,Text;
    int             TamX,TamY,FontSize,PosY;
    QFileInfo       Complete_Name_Entrada;


    if ((argc < 2) || (argc > 3)) {
        printf(" (c) HAM -  JAN 2023\n");
        printf(" COORD_FOTO imagem_original.JPG imagem_saida.JPG\n");
        printf(" Se for informado apenas o nome da imagem original será criado o arquivo (imagem_coord.JPG)\n");
        return (1);
    }

    Complete_Name_Entrada = QFileInfo(argv[1]);

    if (argc == 3) {
        Name_Saida = argv[2];
    } else if (argc == 2) {
        Complete_Name_Entrada = QFileInfo(argv[1]);
        Name_Saida = Complete_Name_Entrada.absolutePath() + "/" +
                     Complete_Name_Entrada.completeBaseName() + "_coord" + ".JPG";
    }

    Name = Complete_Name_Entrada.absoluteFilePath();

    printf(" Arquivo JPG a ser lido: %s\n\n",Name.toStdString().c_str());

    GDALAllRegister();

    GDALDataset *pOldDS;
    pOldDS = (GDALDataset *) GDALOpen(Name.toStdString().c_str(),GA_ReadOnly);
    GDALDataset* poSrcDS = GDALDataset::FromHandle(pOldDS);
    char** papszMetadata = CSLDuplicate(poSrcDS->GetMetadata());



    Lat = getLATfromEXIF(poSrcDS);
    Lon = getLONfromEXIF(poSrcDS);
    strcpy(Lat_Str,poSrcDS->GetMetadataItem("EXIF_GPSLatitude"));
    strcpy(Lon_Str,poSrcDS->GetMetadataItem("EXIF_GPSLongitude"));
    strcpy(Temp,poSrcDS->GetMetadataItem("EXIF_GPSAltitude"));
    RetiraParenteses(GPS_Alt_Str,Temp);
    DecodeXMP(stringList,poSrcDS);
    Relative_Alt = GetXMPItem(stringList,"RelativeAltitude");



    GDALClose(pOldDS);

    Out = "";
    info_H(Name,Out);

    Out = Out + "\n\n XMP: \n";

    for (int i = 0 ; i < stringList.length() ; i++) {
        if (stringList.at(i).contains("dji:"))
                Out = Out + stringList.at(i) + "\n";
    }

    printf("Dados:\n %s",Out.toStdString().c_str());


    QImage image;
    image.load(Name);
    TamX = image.width();
    TamY = image.height();

    FontSize = TamY / 35;

    QPainter painter(&image);
    painter.setPen(QPen(Qt::white));
    painter.setFont(QFont("Times", FontSize, QFont::Bold));
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    PosY =   (1.39 * FontSize);

    QPoint point1 = QPoint( 0, TamY - 2*PosY );
    Text = "Lat=" + QString::number(Lat) + " Lon=" + QString::number(Lon) + " Alt=" + QString::number(Relative_Alt);
    painter.drawText( point1, Text);

    QPoint point2 = QPoint( 0, TamY -  PosY);
    Text = "Lat=" + QString::fromLocal8Bit(Lat_Str) + " Lon=" + QString::fromLocal8Bit(Lon_Str) + " Alt Absoluto=" + QString::fromLocal8Bit(GPS_Alt_Str);
    painter.drawText( point2, Text);

    printf("\n\n Gravando arquivo %s com as coordenadas.\n",Name_Saida.toStdString().c_str());


    image.save(Name_Saida);

    QLabel label;
    label.setPixmap(QPixmap::fromImage(image));
    label.setScaledContents(true);
    label.show();

    return a.exec();
}
