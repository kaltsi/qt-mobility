/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

//TESTED_COMPONENT=src/location

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>

#include <qlandmark.h>
#include <qlandmarkid.h>
#include <qlandmarkcategory.h>
#include <qlandmarkcategoryid.h>
#include <qlandmarkmanager.h>
#include <qgeocoordinate.h>

#include <qlandmarkfilter.h>
#include <qlandmarknamefilter.h>
#include <qlandmarkproximityfilter.h>
#include <qlandmarkcategoryfilter.h>
#include <qlandmarkboxfilter.h>
#include <qlandmarkintersectionfilter.h>
#include <qlandmarkunionfilter.h>
#include <qlandmarkidfilter.h>
#include <qlandmarkattributefilter.h>

#include <qlandmarksortorder.h>
#include <qlandmarknamesort.h>
#include <qlandmarkabstractrequest.h>
#include <qlandmarkidfetchrequest.h>
#include <qlandmarkfetchrequest.h>
#include <qlandmarkfetchbyidrequest.h>
#include <qlandmarksaverequest.h>
#include <qlandmarkremoverequest.h>
#include <qlandmarkcategoryidfetchrequest.h>
#include <qlandmarkcategoryfetchrequest.h>
#include <qlandmarkcategoryfetchbyidrequest.h>
#include <qlandmarkcategorysaverequest.h>
#include <qlandmarkcategoryremoverequest.h>
#include <qlandmarkimportrequest.h>
#include <qlandmarkexportrequest.h>
#include <QMetaType>
#include <QDebug>

#include <QFile>
#include <QList>
#include <QPair>
#include <QVariant>

#ifdef Q_OS_SYMBIAN
#include <EPos_CPosLmDatabaseManager.h>
#else
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#endif

//defines to turn on and off tests
//#define RETRIEVE_CATEGORY
//#define RETRIEVE_LANDMARK
//#define SAVE_CATEGORY
//#define SAVE_LANDMARK
#define REMOVE_CATEGORY

#include <float.h>

QTM_USE_NAMESPACE

Q_DECLARE_METATYPE(QList<QLandmarkCategoryId>);
Q_DECLARE_METATYPE(QList<QLandmarkId>);
Q_DECLARE_METATYPE(QList<QLandmark>);
Q_DECLARE_METATYPE(QList<QLandmarkCategory>);
Q_DECLARE_METATYPE(QLandmarkAbstractRequest::State)
Q_DECLARE_METATYPE(QLandmarkAbstractRequest *)
Q_DECLARE_METATYPE(QLandmarkFetchRequest *)
Q_DECLARE_METATYPE(QLandmarkManager::Error)
Q_DECLARE_METATYPE(QLandmarkSaveRequest *)
Q_DECLARE_METATYPE(QLandmarkRemoveRequest *)
Q_DECLARE_METATYPE(QLandmarkCategorySaveRequest *)
Q_DECLARE_METATYPE(QLandmarkCategoryRemoveRequest *)
Q_DECLARE_METATYPE(QLandmarkCategoryFetchRequest *)

class ManagerListener : public QObject
{
    Q_OBJECT
public slots:
    void landmarksAdded(const QList<QLandmarkId>) {}
    void landmarksChanged(const QList<QLandmarkId>){}
    void landmarksRemoved(const QList<QLandmarkId>){}
    void categoriesAdded(const QList<QLandmarkCategoryId>){}
    void categoriesChanged(const QList<QLandmarkCategoryId>){}
    void categoriesRemoved(const QList<QLandmarkCategoryId>){}
};

class tst_QLandmarkManager : public QObject
{
    Q_OBJECT
public:
    tst_QLandmarkManager() {
    qRegisterMetaType<QList<QLandmarkCategoryId> >();
    qRegisterMetaType<QList<QLandmarkId> >();
    qRegisterMetaType<QList<QLandmark> >();
    qRegisterMetaType<QList<QLandmarkCategory> >();
    qRegisterMetaType<QLandmarkAbstractRequest *>();
    qRegisterMetaType<QLandmarkFetchRequest *>();
    qRegisterMetaType<QLandmarkSaveRequest *>();
    qRegisterMetaType<QLandmarkRemoveRequest *>();
    qRegisterMetaType<QLandmarkCategoryFetchRequest *>();
    qRegisterMetaType<QLandmarkCategorySaveRequest *>();
    qRegisterMetaType<QLandmarkCategoryRemoveRequest *>();
    qRegisterMetaType<QLandmarkManager::Error>();
    qRegisterMetaType<QLandmarkAbstractRequest::State>();

    exportFile = "exportfile";
    QFile::remove(exportFile);

#ifdef Q_OS_SYMBIAN
    deleteDefaultDb();
#else
    QFile::remove("test.db");
    //TODO: verify if this is needed
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","landmarks");
    db.setDatabaseName("test.db");
#endif
    }

private:
    QLandmarkManager *m_manager;
    ManagerListener *m_listener;
    QString exportFile;

    bool waitForAsync(QSignalSpy &spy, QLandmarkAbstractRequest *request,
                    QLandmarkManager::Error error = QLandmarkManager::NoError,
                    int ms=1500, QLandmarkAbstractRequest::State state = QLandmarkAbstractRequest::FinishedState) {
        bool ret = true;
        int msWaitedSoFar =0;
        while(msWaitedSoFar < ms) {
            QTest::qWait(100);
            msWaitedSoFar +=100;
            if (spy.count() ==2)
                break;
        }

        QTest::qWait(ms);
        if (spy.count() != 2) {
            qWarning() << "Spy count mismatch, expected = " << 2 << ", actual = " << spy.count();
            ret = false;
        }
        spy.clear();

        if (request->error() != error) {
            qWarning() << "Error mismatch, expected = " << error
                                       << ", actual=" << request->error();
            qWarning() << "Error string =" << request->errorString();
            ret = false;
        }

        if (error == QLandmarkManager::NoError && !request->errorString().isEmpty()) {
            qWarning() << "Error string is not empty as expected, error string = " << request->errorString();
            ret = false;
        }

        if (request->state() != state) {
            qWarning() << "Request State mismatch, expected = " << state
                                               << ", actual =" << request->state();
            ret = false;
        }
        return ret;
    }

    bool doCategoryFetch(const QString type, const QList<QLandmarkCategoryId> &ids, QList<QLandmarkCategory> *cats, QLandmarkManager::Error expectedError = QLandmarkManager::NoError) {
        bool result = false;

        if (type== "sync") {
            *cats = m_manager->categories();
            result = (m_manager->error() == expectedError);
        }  else if (type == "async") {
            QLandmarkCategoryFetchByIdRequest fetchRequest(m_manager);
            QSignalSpy spy(&fetchRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
            fetchRequest.setCategoryIds(ids);
            fetchRequest.start();
            bool waitResult = waitForAsync(spy, &fetchRequest,expectedError,100);
            if (!waitResult)
                qWarning("Wait for async failed for category fetch");

            *cats = fetchRequest.categories();
            result = result && waitResult;
        } else {
            qFatal("Unknown test row type");
        }
        return result;
    }

    bool doCategoryFetch(const QString type, int limit, int offset, const QLandmarkNameSort &nameSort, QList<QLandmarkCategory> *cats,
                         const QLandmarkManager::Error &error){
        bool result =false;
        if (type== "sync") {
            *cats = m_manager->categories(limit,offset, nameSort);
            bool syncResult = m_manager->error() == error;
            if (!syncResult)
                qWarning() << "sync category fetch error code did not match expected = " << error << " Actual = " << m_manager->error();
            QList<QLandmarkCategoryId> catIds = m_manager->categoryIds(limit, offset, nameSort);
            bool syncIdResult = m_manager->error() == error;
            if (!syncIdResult)
                qWarning() << "sync category id fetch error code did not match expected = " << error << " Actual = " << m_manager->error();

            bool checkIdsResult = checkIds(*cats,catIds);
            if (!checkIdsResult)
                qWarning("sync category id fetch failed to match sync category fetch");
            result = syncResult && syncIdResult && checkIdsResult;
            return result;
        } else if (type == "async") {
            QLandmarkCategoryFetchRequest fetchRequest(m_manager);
            QSignalSpy spy(&fetchRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
            fetchRequest.setLimit(limit);
            fetchRequest.setOffset(offset);
            fetchRequest.setSorting(nameSort);
            fetchRequest.start();

            bool waitResult = waitForAsync(spy, &fetchRequest,error,100);
            if (!waitResult)
                    qWarning("Wait for async failed for category fetch");
            result = waitResult;
            *cats = fetchRequest.categories();

            QLandmarkCategoryIdFetchRequest idFetchRequest(m_manager);
            QSignalSpy spyId(&idFetchRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
            idFetchRequest.setLimit(limit);
            idFetchRequest.setOffset(offset);
            idFetchRequest.setSorting(nameSort);
            idFetchRequest.start();
            bool waitIdResult = waitForAsync(spyId, &idFetchRequest,error,100);
            if (!waitIdResult)
                qWarning("Wait for async failed for category id fetch");
            QList<QLandmarkCategoryId> catIds = idFetchRequest.categoryIds();
            bool checkIdsResult = checkIds(*cats, catIds);
            if (!checkIdsResult)
                qWarning("async category id fetch failed to match async category fetch");
            result = result && waitIdResult && checkIdsResult;
        } else {
            qFatal("Unknown test row type");
        }
    return result;
    }

    bool doFetch(const QString type, const QLandmarkFilter &filter, QList<QLandmark> *lms,
                 QLandmarkManager::Error error = QLandmarkManager::NoError){
        bool result =false;
        if (type== "sync") {
            *lms = m_manager->landmarks(filter);
            bool syncResult = (m_manager->error() == error);
            if (!syncResult)
                qWarning() << "sync landmark fetch error code did not match expected error = " << error << "Actual =" << m_manager->error();

            QList<QLandmarkId> lmIds = m_manager->landmarkIds(filter);
            bool syncIdResult = (m_manager->error() == error);
            if (!syncIdResult)
                qWarning() << "sync landmark id fetch error code did not match expected error = " << error << "Actual =" << m_manager->error();

            bool checkIdsResult = checkIds(*lms,lmIds);
            if (!checkIdsResult)
                qWarning("sync landmark id fetch failed to match sync landmark fetch");
            result = syncResult && syncIdResult && checkIdsResult;
        } else if (type == "async") {
            QLandmarkFetchRequest fetchRequest(m_manager);
            QSignalSpy spy(&fetchRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
            fetchRequest.setFilter(filter);
            fetchRequest.start();
            bool waitResult = waitForAsync(spy, &fetchRequest, error,100);
            if (!waitResult)
                    qWarning("Wait for async failed for landmark fetch");
            result = waitResult;
            *lms = fetchRequest.landmarks();
            QLandmarkIdFetchRequest idFetchRequest(m_manager);
            QSignalSpy spyId(&idFetchRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
            idFetchRequest.setFilter(filter);
            idFetchRequest.start();
            waitResult = waitForAsync(spyId, &idFetchRequest, error,100);
            if (!waitResult)
                qWarning("Wait for async failed for landmark id fetch");
            QList<QLandmarkId> lmIds = idFetchRequest.landmarkIds();
            bool checkIdsResult = checkIds(*lms, lmIds);
            if (!checkIdsResult)
                qWarning("async landmark id fetch failed to match async landmark fetch");
            result = result && waitResult && checkIdsResult;
        } else {
            qFatal("Unknown test row type");
        }
        return result;
    }

     bool doFetchById(const QString type, const QList<QLandmarkId>lmIds, QList<QLandmark> *lms,
                        QLandmarkManager::Error error = QLandmarkManager::NoError){
        bool result =false;
        if (type== "sync") {
            *lms = m_manager->landmarks(lmIds);
            result = (m_manager->error() == error);
        } else if (type == "async") {
            QLandmarkFetchByIdRequest fetchRequest(m_manager);
            QSignalSpy spy(&fetchRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
            fetchRequest.setLandmarkIds(lmIds);
            fetchRequest.start();
            result = waitForAsync(spy, &fetchRequest,error,100);
            *lms = fetchRequest.landmarks();
        } else {
            qFatal("Unknown test row type");
        }
        return result;
    }

     bool doSingleCategorySave(const QString &type, QLandmarkCategory *category,
                 QLandmarkManager::Error error = QLandmarkManager::NoError)
     {
         bool result = false;
         if (type == "sync") {
             if (error == QLandmarkManager::NoError) {
                 result = m_manager->saveCategory(category)
                          && (m_manager->error() == QLandmarkManager::NoError);
             } else {
                 result = (!m_manager->saveCategory(category))
                          && (m_manager->error() == error);
             }
             if (!result) {
                 qWarning() << "Actual error = " << m_manager->error();
                 qWarning() << "Actual error string = " << m_manager->errorString();
                 qWarning() << "Expected error=" << error;
             }
         } else if (type == "async") {
             QLandmarkCategorySaveRequest catSaveRequest(m_manager);
             QSignalSpy spy(&catSaveRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
             QSignalSpy resultsAvailableSpy(&catSaveRequest, SIGNAL(resultsAvailable()));
             catSaveRequest.setCategory(*category);
             catSaveRequest.start();
             result = waitForAsync(spy, &catSaveRequest,error,1000);
             if (!result) {
                 qWarning() << "Wait for asynchronous request failed";
             }

             QMap<int, QLandmarkManager::Error> errorMap;
             errorMap = catSaveRequest.errorMap();
             result = result && (catSaveRequest.categories().count() ==1);

             if (error == QLandmarkManager::NoError) {
                 result = result && (errorMap.count() ==0);
             } else {
                 result = result && (errorMap.count() == 1)
                           && (errorMap.value(0) == error);
             }
             if (!result) {
                 qWarning() << "Actual error " << catSaveRequest.error();
                 qWarning() << "Actual error string = " << catSaveRequest.errorString();
                 qWarning() << "Expected error = " << error;
             }

             *category = catSaveRequest.categories().at(0);
         } else {
             qFatal("Unknown test row type");
         }
         return result;
     }

     bool doSingleLandmarkSave(const QString &type, QLandmark *landmark,
                 QLandmarkManager::Error error = QLandmarkManager::NoError)
     {
         bool result = false;
         if (type == "sync") {
             if (error == QLandmarkManager::NoError) {
                 result = m_manager->saveLandmark(landmark)
                          && (m_manager->error() == QLandmarkManager::NoError);
             } else {
                 result = (!m_manager->saveLandmark(landmark))
                          && (m_manager->error() == error);
             }
         } else if (type == "async") {
             QLandmarkSaveRequest lmSaveRequest(m_manager);
             QSignalSpy spy(&lmSaveRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
             QSignalSpy resultsAvailableSpy(&lmSaveRequest, SIGNAL(resultsAvailable()));
             lmSaveRequest.setLandmark(*landmark);
             lmSaveRequest.start();
             result = waitForAsync(spy, &lmSaveRequest,error,1000);
             result = result && (resultsAvailableSpy.count()==1);
             QMap<int, QLandmarkManager::Error> errorMap;
             errorMap = lmSaveRequest.errorMap();
             result = result && (lmSaveRequest.landmarks().count() ==1);
             if (error == QLandmarkManager::NoError) {
                 result = result && (errorMap.count() ==0);
             } else {
                 result = result && (errorMap.count() == 1)
                           && (errorMap.value(0) == error);
             }
             *landmark = lmSaveRequest.landmarks().at(0);
         } else {
             qFatal("Unknown test row type");
         }
         return result;
     }

     bool doSave(const QString &type, QList<QLandmark> *lms,
                 QLandmarkManager::Error error = QLandmarkManager::NoError,
                 QMap<int,QLandmarkManager::Error> *errorMap = 0)
     {
         bool result = false;
         if (type == "sync") {
             if (error == QLandmarkManager::NoError) {
                 result = m_manager->saveLandmarks(lms,errorMap)
                          && (m_manager->error() == QLandmarkManager::NoError);
             } else {
                 result = (!m_manager->saveLandmarks(lms,errorMap))
                          && (m_manager->error() == error);
             }
         } else if (type == "async") {
             QLandmarkSaveRequest saveRequest(m_manager);
             QSignalSpy spy(&saveRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
             saveRequest.setLandmarks(*lms);
             saveRequest.start();
             result = waitForAsync(spy, &saveRequest,error,1000);
             if (errorMap)
                *errorMap = saveRequest.errorMap();
             *lms = saveRequest.landmarks();
         } else {
             qFatal("Unknown test row type");
         }
         return result;
     }

     bool doSingleCategoryRemove(const QString &type, const QLandmarkCategoryId &categoryId,
                 QLandmarkManager::Error error = QLandmarkManager::NoError)
     {
         bool result = false;
         if (type == "sync") {
             if (error == QLandmarkManager::NoError) {
                 result = m_manager->removeCategory(categoryId)
                          && (m_manager->error() == QLandmarkManager::NoError);
             } else {
                 result = (!m_manager->removeCategory(categoryId))
                          && (m_manager->error() == error);
             }
             if (!result) {
                 qWarning() << "Actual error = " << m_manager->error();
                 qWarning() << "Actual error string = " << m_manager->errorString();
                 qWarning() << "Expected error=" << error;
             }
         } else if (type == "async") {
             QLandmarkCategoryRemoveRequest catRemoveRequest(m_manager);
             QSignalSpy spy(&catRemoveRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
             QSignalSpy resultsAvailableSpy(&catRemoveRequest, SIGNAL(resultsAvailable()));
             catRemoveRequest.setCategoryId(categoryId);
             catRemoveRequest.start();
             result = waitForAsync(spy, &catRemoveRequest,error,1000);

             bool alreadyWarned = false;
             if (!result) {
                 qWarning() << "Wait for asynchronous request failed";
                 alreadyWarned = true;
             }

             QMap<int, QLandmarkManager::Error> errorMap;
             errorMap = catRemoveRequest.errorMap();


             if (error == QLandmarkManager::NoError) {
                 result = result && (errorMap.count() ==0);
                 if (errorMap.count() !=0) {
                     qWarning() << "Expected error map count of 0";
                     alreadyWarned = true;
                 }
             } else {
                 result = result && (errorMap.count() == 1)
                           && (errorMap.value(0) == error);
                 if (errorMap.count() !=1) {
                     qWarning() << "Expected errorMap count of 0";
                     alreadyWarned = true;
                 }
             }
             if (!result && !alreadyWarned) {
                 qWarning() << "Actual error " << catRemoveRequest.error();
                 qWarning() << "Actual error string = " << catRemoveRequest.errorString();
                 qWarning() << "Expected error = " << error;
             }
         } else {
             qFatal("Unknown test row type");
         }
         return result;
     }

     bool doSingleLandmarkRemove(const QString &type, const QLandmarkId &landmarkId,
                 QLandmarkManager::Error error = QLandmarkManager::NoError)
     {
         bool result = false;
         if (type == "sync") {
             if (error == QLandmarkManager::NoError) {
                 result = m_manager->removeLandmark(landmarkId)
                          && (m_manager->error() == QLandmarkManager::NoError);
             } else {
                 result = (!m_manager->removeLandmark(landmarkId))
                          && (m_manager->error() == error);
             }
             if (!result) {
                 qWarning() << "Actual error = " << m_manager->error();
                 qWarning() << "Actual error string = " << m_manager->errorString();
                 qWarning() << "Expected error=" << error;
             }
         } else if (type == "async") {
             QLandmarkRemoveRequest lmRemoveRequest(m_manager);
             QSignalSpy spy(&lmRemoveRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
             QSignalSpy resultsAvailableSpy(&lmRemoveRequest, SIGNAL(resultsAvailable()));
             lmRemoveRequest.setLandmarkId(landmarkId);
             lmRemoveRequest.start();
             result = waitForAsync(spy, &lmRemoveRequest,error,1000);

             bool alreadyWarned = false;
             if (!result) {
                 qWarning() << "Wait for asynchronous request failed";
                 alreadyWarned = true;
             }

             QMap<int, QLandmarkManager::Error> errorMap;
             errorMap = lmRemoveRequest.errorMap();

             if (error == QLandmarkManager::NoError) {
                 result = result && (errorMap.count() ==0);
                 if (errorMap.count() !=0) {
                     qWarning() << "Expected error map count of 0";
                     alreadyWarned = true;
                 }
             } else {
                 result = result && (errorMap.count() == 1)
                           && (errorMap.value(0) == error);
                 if (errorMap.count() !=1) {
                     qWarning() << "Expected errorMap count of 0";
                     alreadyWarned = true;
                 }
             }
             if (!result && !alreadyWarned) {
                 qWarning() << "Actual error " << lmRemoveRequest.error();
                 qWarning() << "Actual error string = " << lmRemoveRequest.errorString();
                 qWarning() << "Expected error = " << error;
             }
         } else {
             qFatal("Unknown test row type");
         }
         return result;
     }

     bool doRemove(const QString &type, QList<QLandmarkId> lmIds,
                 QLandmarkManager::Error error = QLandmarkManager::NoError,
                 QMap<int,QLandmarkManager::Error> *errorMap = 0)
     {
         bool result = false;
         if (type == "sync") {
             if (error == QLandmarkManager::NoError) {
                 result = m_manager->removeLandmarks(lmIds,errorMap)
                          && (m_manager->error() == QLandmarkManager::NoError);
             } else {
                 result = (!m_manager->removeLandmarks(lmIds,errorMap))
                          && (m_manager->error() == error);
             }
         } else if (type == "async") {
             QLandmarkRemoveRequest removeRequest(m_manager);
             QSignalSpy spy(&removeRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
             removeRequest.setLandmarkIds(lmIds);
             removeRequest.start();
             result = waitForAsync(spy, &removeRequest,error,200);
             if (errorMap)
                 *errorMap = removeRequest.errorMap();
         } else {
             qFatal("Unknown test row type");
         }
         return result;
     }

    bool doImport(const QString &type, const QString filename,
                  QLandmarkManager::Error error =QLandmarkManager::NoError){
        bool result =false;
        if (type== "sync") {
            if (error == QLandmarkManager::NoError) {
                result = (m_manager->importLandmarks(filename))
                         && (m_manager->error() == QLandmarkManager::NoError);
            } else {
                result = (!m_manager->importLandmarks(filename))
                        && (m_manager->error() == error);
            }
        } else if (type == "async") {
            QLandmarkImportRequest importRequest(m_manager);
            QSignalSpy spy(&importRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
            importRequest.setFileName(filename);
            importRequest.start();
            result = waitForAsync(spy, &importRequest,error,150);
        } else {
            qFatal("Unknown test row type");
        }
        return result;
    }

    bool checkCategoryCount(int expectedCount) {
        return (m_manager->categories().count() == expectedCount)
                && checkIds(m_manager->categories(),m_manager->categoryIds());
    }

    bool checkLandmarkCount(int expectedCount) {
        return (m_manager->landmarks().count() == expectedCount)
                && checkIds(m_manager->landmarks(), m_manager->landmarkIds());
    }

    bool checkIds(const QList<QLandmark> &lms, const QList<QLandmarkId> &lmIds)
    {
        if (lms.count() != lmIds.count())
            return false;
        for (int i=0; i < lms.count(); ++i) {
            if (lms.at(i).landmarkId() != lmIds.at(i)) {
                return false;
            }
        }
        return true;
    }

        bool checkIds(const QList<QLandmarkCategory> &cats, const QList<QLandmarkCategoryId> &catIds)
    {
        if (cats.count() != catIds.count())
            return false;
        for (int i=0; i < cats.count(); ++i) {
            if (cats.at(i).categoryId() != catIds.at(i)) {
                return false;
            }
        }
        return true;
    }

    //checks if an id fetch request will return the ids for the same landmarks as in \a lms
    bool checkIdFetchRequest(const QList<QLandmark> &lms, const QLandmarkFilter &filter,
                            const QList<QLandmarkSortOrder> &sorting = QList<QLandmarkSortOrder>(),
                            int limit =-1, int offset=0) {
        //check that the id request will return the same results
        QLandmarkIdFetchRequest idFetchRequest(m_manager);
        idFetchRequest.setFilter(filter);
        idFetchRequest.setSorting(sorting);
        idFetchRequest.setLimit(limit);
        idFetchRequest.setOffset(offset);

        QSignalSpy spyId(&idFetchRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
        idFetchRequest.start();

        if (!waitForAsync(spyId, &idFetchRequest))
            return false;
        QList<QLandmarkId> lmIds = idFetchRequest.landmarkIds();
        if (lmIds.count() != lms.count())
            return false;
        for (int i=0; i < lmIds.count(); ++i) {
            if( !(lms.at(i).landmarkId() == lmIds.at(i)))
                return false;
        }

        return true;
    }

    bool checkIdFetchRequest(const QList<QLandmark> &lms, const QLandmarkFilter &filter,
                            const QLandmarkSortOrder &sorting,
                            int limit=-1, int offset=0)
    {
        QList<QLandmarkSortOrder> sortOrders;
        sortOrders << sorting;
        return  checkIdFetchRequest(lms, filter, sortOrders, limit, offset);
    }

    bool checkCategoryIdFetchRequest(const QList<QLandmarkCategory> &cats, const QLandmarkNameSort &nameSort)
    {
        QLandmarkCategoryIdFetchRequest catIdFetchRequest(m_manager);
        catIdFetchRequest.setSorting(nameSort);
        QSignalSpy spyCatId(&catIdFetchRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
        catIdFetchRequest.start();
        if (!waitForAsync(spyCatId, &catIdFetchRequest))
            return false;
         QList<QLandmarkCategoryId> catIds = catIdFetchRequest.categoryIds();
         if (catIds.count() != cats.count())
            return false;
         for (int i=0; i < catIds.count(); ++i) {
            if (!(cats.at(i).categoryId() == catIds.at(i)))
                return false;
         }
        return true;
    }

        //ensure connectNotify is called by m_manager
    void connectNotifications() {
        Q_ASSERT(m_manager);
        Q_ASSERT(m_listener);
        connect(m_manager, SIGNAL(landmarksAdded(QList<QLandmarkId>)),
                m_listener, SLOT(landmarksAdded(QList<QLandmarkId>)));
        connect(m_manager, SIGNAL(landmarksChanged(QList<QLandmarkId>)),
                m_listener, SLOT(landmarksChanged(QList<QLandmarkId>)));
        connect(m_manager, SIGNAL(landmarksRemoved(QList<QLandmarkId>)),
                m_listener, SLOT(landmarksRemoved(QList<QLandmarkId>)));
        connect(m_manager, SIGNAL(categoriesAdded(QList<QLandmarkCategoryId>)),
                m_listener,SLOT(categoriesAdded(QList<QLandmarkCategoryId>)));
        connect(m_manager, SIGNAL(categoriesChanged(QList<QLandmarkCategoryId>)),
                m_listener,SLOT(categoriesChanged(QList<QLandmarkCategoryId>)));
        connect(m_manager, SIGNAL(categoriesRemoved(QList<QLandmarkCategoryId>)),
                m_listener, SLOT(categoriesRemoved(QList<QLandmarkCategoryId>)));
     }

      void disconnectNotifications() {
        Q_ASSERT(m_manager);
        Q_ASSERT(m_listener);
        disconnect(m_manager, SIGNAL(landmarksAdded(QList<QLandmarkId>)),
                m_listener, SLOT(landmarksAdded(QList<QLandmarkId>)));
        disconnect(m_manager, SIGNAL(landmarksChanged(QList<QLandmarkId>)),
                m_listener, SLOT(landmarksChanged(QList<QLandmarkId>)));
        disconnect(m_manager, SIGNAL(landmarksRemoved(QList<QLandmarkId>)),
                m_listener, SLOT(landmarksRemoved(QList<QLandmarkId>)));
        disconnect(m_manager, SIGNAL(categoriesAdded(QList<QLandmarkCategoryId>)),
                m_listener,SLOT(categoriesAdded(QList<QLandmarkCategoryId>)));
        disconnect(m_manager, SIGNAL(categoriesChanged(QList<QLandmarkCategoryId>)),
                m_listener,SLOT(categoriesChanged(QList<QLandmarkCategoryId>)));
        disconnect(m_manager, SIGNAL(categoriesRemoved(QList<QLandmarkCategoryId>)),
                m_listener, SLOT(categoriesRemoved(QList<QLandmarkCategoryId>)));
     }

    void createDb() {
        if (m_manager != 0) {
            delete m_manager;
            m_manager =0;
        }

        if (m_listener !=0) {
            delete m_listener;
            m_listener =0;
        }
        QMap<QString, QString> map;
#ifdef Q_OS_SYMBIAN
        m_manager = new QLandmarkManager();
#else

        map["filename"] = "test.db";
        m_manager = new QLandmarkManager("com.nokia.qt.landmarks.engines.sqlite", map);
#endif
        m_listener = new ManagerListener;
        connectNotifications();
    }

#ifdef Q_OS_SYMBIAN
    void deleteDefaultDb(){
        CPosLmDatabaseManager* lmDbManager = CPosLmDatabaseManager::NewL();
        CleanupStack::PushL(lmDbManager);
        HBufC* defaultDbUri = lmDbManager->DefaultDatabaseUriLC();
        if (lmDbManager->DatabaseExistsL(defaultDbUri->Des())) {
            lmDbManager->DeleteDatabaseL(defaultDbUri->Des());
        }
        CleanupStack::PopAndDestroy(defaultDbUri);
        CleanupStack::PopAndDestroy(lmDbManager);
    }
#endif

    void deleteDb() {
        QFile file;

#ifdef Q_OS_SYMBIAN
        QList<QLandmarkId> lmIds = m_manager->landmarkIds();
        for(int i=0; i < lmIds.count(); ++i) {
            QVERIFY(m_manager->removeLandmark(lmIds.at(i)));
        }

        QList<QLandmarkCategoryId> catIds = m_manager->categoryIds();
        for (int i=0; i < catIds.count(); ++i) {
            if (!m_manager->isReadOnly(catIds.at(i)))
                QVERIFY(m_manager->removeCategory(catIds.at(i)));
        }
#else
        {
            QSqlDatabase db = QSqlDatabase::database("landmarks");
            QSqlQuery q1("delete from landmark;", db);
            q1.exec();
            QSqlQuery q2("delete from category;", db);
            q2.exec();
            QSqlQuery q3("delete from landmark_category;", db);
            q3.exec();
        }

        QFile::remove("test.db");
#endif
        delete m_manager;
        m_manager = 0;

        delete m_listener;
        m_listener =0;

        QFile::remove(exportFile);
        file.setFileName("nopermfile");
        file.setPermissions(QFile::WriteOwner | QFile::WriteUser | QFile::WriteOther);
        file.remove();
    }

#ifdef Q_OS_SYMBIAN
    void clearDb() {

        QList<QLandmarkId> lmIds = m_manager->landmarkIds();
        for(int i=0; i < lmIds.count(); ++i)
            QVERIFY(m_manager->removeLandmark(lmIds.at(i)));

        QList<QLandmarkCategoryId> catIds = m_manager->categoryIds();
        for (int i=0; i < catIds.count(); ++i) {
            if (!m_manager->isReadOnly(catIds.at(i)))
                QVERIFY(m_manager->removeCategory(catIds.at(i)));
        }

        QTest::qWait(20);//try ensure notifications for these deletions
                         //are made prior to each test function
    }
#endif


#ifndef Q_OS_SYMBIAN
    bool tablesExist() {
        QStringList tables = QSqlDatabase::database("landmarks").tables();
        tables.sort();

        QStringList knownTables;
        knownTables << "category";
        knownTables << "category_attribute";
        knownTables << "category_notification";
        knownTables << "landmark";
        knownTables << "landmark_attribute";
        knownTables << "landmark_category";
        knownTables << "landmark_notification";
        knownTables << "version";
        return (tables == knownTables);
    }
#endif

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

#ifndef Q_OS_SYMBIAN
    void createDbNew();
    void createDbExists();
#endif

    void invalidManager();

    void retrieveCategory();
    void retrieveCategory_data();

#ifdef RESTORE
    void categoryFetchCancelAsync();
#endif

    void retrieveLandmark();
    void retrieveLandmark_data();

    /* TODO: restore
    void asyncLandmarkFetchCancel();
*/
    void saveCategory();
    void saveCategory_data();

    void saveLandmark();
    void saveLandmark_data();

    void removeCategory();
    void removeCategory_data();

    void removeLandmark();
    void removeLandmark_data();

    void categories();
    void categories_data();

/*
    void filterLandmarksLimitMatches();
    void filterLandmarksLimitMatchesAsync();
*/

    void filterLandmarksDefault();
    void filterLandmarksDefault_data();

    void filterLandmarksName();
    void filterLandmarksName_data();

    void filterLandmarksProximity();
    void filterLandmarksProximity_data();

    void filterLandmarksProximityOrder();
    void filterLandmarksProximityOrder_data();

    void filterLandmarksCategory();
    void filterLandmarksCategory_data();


    void filterLandmarksBox();
    void filterLandmarksBox_data();


    void filterLandmarksIntersection();
    void filterLandmarksIntersection_data();

    void filterLandmarksMultipleBox();
    void filterLandmarksMultipleBox_data();


    void filterLandmarksUnion();
    void filterLandmarksUnion_data();


    void filterAttribute();
    void filterAttribute_data();


    void sortLandmarksNull();

    void sortLandmarksName();
    void sortLandmarksNameAsync();

    void importGpx();
    void importGpx_data();

    void importLmx();
    void importLmx_data();

    void exportGpx();
    void exportGpx_data();


    void exportLmx();
    void exportLmx_data();

    void importFile();
    void importFile_data();

    void supportedFormats();

    void filterSupportLevel();
    void sortOrderSupportLevel();

    void isFeatureSupported();


    void categoryLimitOffset();
    //TODO: void categoryLimitOffsetAsync()

    void notificationCheck();
    void testConvenienceFunctions();
};


void tst_QLandmarkManager::initTestCase() {
    m_manager = 0;
    m_listener = 0;
}

void tst_QLandmarkManager::init() {
    createDb();
#ifdef Q_OS_SYMBIAN
    clearDb();
#endif
}

void tst_QLandmarkManager::cleanup() {
    deleteDb();
}

void tst_QLandmarkManager::cleanupTestCase() {
    QFile::remove(exportFile);
}

#ifndef Q_OS_SYMBIAN
void tst_QLandmarkManager::createDbNew() {
    QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    QVERIFY(tablesExist());

    deleteDb();
    createDb();
    QCOMPARE(m_manager->error(), QLandmarkManager::NoError);

    QVERIFY(tablesExist());
}

void tst_QLandmarkManager::createDbExists() {
    QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    QVERIFY(tablesExist());

    createDb();

    QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    QVERIFY(tablesExist());
}
#endif

void tst_QLandmarkManager::invalidManager()
{
    QLandmarkManager manager("does.not.exist");
    QVERIFY(manager.error() == QLandmarkManager::InvalidManagerError);

    QLandmark lm;
    lm.setName("LM");
    QVERIFY(!manager.saveLandmark(&lm));
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QList<QLandmark> lms;
    lms.append(lm);
    QVERIFY(!manager.saveLandmarks(&lms));
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QLandmarkId lmId;
    QVERIFY(!manager.removeLandmark(lmId));
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QList<QLandmarkId> lmIds;
    lmIds.append(lmId);
    QVERIFY(!manager.removeLandmarks(lmIds));
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QLandmarkCategory cat;
    cat.setName("cat");
    QVERIFY(!manager.saveCategory(&cat));
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QLandmarkCategoryId catId;
    QVERIFY(!manager.removeCategory(catId));
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QCOMPARE(manager.category(catId), QLandmarkCategory());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QCOMPARE(manager.categories(), QList<QLandmarkCategory>());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QCOMPARE(manager.categoryIds(), QList<QLandmarkCategoryId>());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QList<QLandmarkCategoryId> catIds;
    catIds << catId;
    QCOMPARE(manager.categories(catIds), QList<QLandmarkCategory>());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QCOMPARE(manager.landmark(lmId), QLandmark());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QCOMPARE(manager.landmarks(lmIds), QList<QLandmark>());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QCOMPARE(manager.landmarks(), QList<QLandmark>());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QCOMPARE(manager.landmarkIds(), QList<QLandmarkId>());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QVERIFY(!manager.importLandmarks("test.gpx"));
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QVERIFY(!manager.exportLandmarks("test.gpx", QLandmarkManager::Gpx));
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QCOMPARE(manager.supportedFormats(QLandmarkManager::ImportOperation), QStringList());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QVERIFY(!manager.isFeatureSupported(QLandmarkManager::NotificationsFeature));
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QLandmarkFilter filter;
    QCOMPARE(manager.filterSupportLevel(filter), QLandmarkManager::NoSupport);
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QVERIFY(manager.isReadOnly());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QVERIFY(manager.isReadOnly(lmId));
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QVERIFY(manager.isReadOnly(catId));
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QCOMPARE(manager.searchableLandmarkAttributeKeys(), QStringList());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QCOMPARE(manager.managerName(), QString());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QMap<QString,QString> stringMap;
    QVERIFY(manager.managerParameters() == stringMap);
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QCOMPARE(manager.managerUri(), QString());
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

    QCOMPARE(manager.managerVersion(), 0);
    QCOMPARE(manager.error(), QLandmarkManager::InvalidManagerError);

#ifndef Q_OS_SYMBIAN
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","landmarkstest");
    db.setDatabaseName("test2.db");

    db.open();
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS landmark( "
            "id INTEGER PRIMARY KEY, "
            "name TEXT, "
            "latitude REAL, "
            "longitude REAL, "
            " altitude REAL);");

    QMap<QString,QString> parameters;
    parameters.insert("filename", "test2.db");
    QLandmarkManager manager2 ("com.nokia.qt.landmarks.engines.sqlite",parameters);
    QVERIFY(manager2.error() == QLandmarkManager::VersionMismatchError);
    QFile::remove("test2.db");
#endif
}

void tst_QLandmarkManager::retrieveCategory() {
    QFETCH(QString, type);
    QLandmarkCategoryId id1;

    QLandmarkCategoryFetchByIdRequest catFetchByIdRequest(m_manager);
    QSignalSpy spy(&catFetchByIdRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
    QList<QLandmarkCategory> cats;

    if ( type == "sync") {
        //try a default constructed category id
        QCOMPARE(m_manager->category(QLandmarkCategoryId()), QLandmarkCategory());
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(m_manager->category(QLandmarkCategoryId()).categoryId().isValid(), false);

        //try an id does not exist
        id1.setManagerUri(m_manager->managerUri());
        id1.setLocalId("100");

        QCOMPARE(m_manager->category(id1), QLandmarkCategory());
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(m_manager->category(id1).categoryId().isValid(), false);
    } else if (type == "async") {
        //try a default constructed id
        catFetchByIdRequest.setCategoryId(QLandmarkCategoryId());
        catFetchByIdRequest.start();
        QVERIFY(waitForAsync(spy, &catFetchByIdRequest,QLandmarkManager::DoesNotExistError,100));
        cats = catFetchByIdRequest.categories();
        QCOMPARE(cats.count(), 1);
        QCOMPARE(cats.at(0), QLandmarkCategory());
        QCOMPARE(cats.at(0).categoryId().isValid(), false);

        //try an id that does not exist
        id1.setManagerUri(m_manager->managerUri());
        id1.setLocalId("100");
        catFetchByIdRequest.setCategoryId(id1);
        catFetchByIdRequest.start();
        QVERIFY(waitForAsync(spy, &catFetchByIdRequest,QLandmarkManager::DoesNotExistError,100));
        cats = catFetchByIdRequest.categories();
        QCOMPARE(cats.count(), 1);
        QCOMPARE(cats.at(0), QLandmarkCategory());
        QCOMPARE(cats.at(0).categoryId().isValid(), false);
    } else {
        qFatal("Unknown test row type");
    }

    QLandmarkCategory cat2;
    cat2.setName("CAT2");
    cat2.setIconUrl(QUrl("CAT2Icon"));
    QVERIFY(m_manager->saveCategory(&cat2));

    QLandmarkCategoryId id2;

    if ( type == "sync") {
        //try a manager that doesn't match
        id2.setManagerUri("wrong.manager");
        id2.setLocalId(cat2.categoryId().localId());
        QCOMPARE(m_manager->category(id2), QLandmarkCategory());
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(m_manager->category(id2).categoryId().isValid(), false);

        //try an existing id
        id2.setManagerUri(cat2.categoryId().managerUri());
        id2.setLocalId(cat2.categoryId().localId());
        QCOMPARE(m_manager->category(id2), cat2);
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
        QCOMPARE(m_manager->category(id2).categoryId().isValid(), true);

        //ensure consecutive calls clears the error
        QCOMPARE(m_manager->category(id1), QLandmarkCategory());
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(m_manager->category(id2), cat2);
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);


    } else if (type == "async") {
        //try a manager that doesn't match
        id2.setManagerUri("wrong.manager");
        id2.setLocalId(cat2.categoryId().localId());
        catFetchByIdRequest.setCategoryId(id2);
        catFetchByIdRequest.start();
        QVERIFY(waitForAsync(spy, &catFetchByIdRequest,QLandmarkManager::DoesNotExistError,100));
        cats = catFetchByIdRequest.categories();
        QCOMPARE(cats.count(), 1);
        QCOMPARE(cats.at(0), QLandmarkCategory());
        QCOMPARE(cats.at(0).categoryId().isValid(), false);

        //try an existing id
        id2.setManagerUri(cat2.categoryId().managerUri());
        id2.setLocalId(cat2.categoryId().localId());
        catFetchByIdRequest.setCategoryId(id2);
        catFetchByIdRequest.start();
        QVERIFY(waitForAsync(spy, &catFetchByIdRequest,QLandmarkManager::NoError,100));
        cats = catFetchByIdRequest.categories();
        QCOMPARE(cats.count(), 1);
        QCOMPARE(cats.at(0), cat2);
        QCOMPARE(cats.at(0).categoryId().isValid(), true);
    } else {
        qFatal("Unknown test row type");
    }

    if (type == "async") {
        //try retrive multiple categories
        QLandmarkCategory catA;
        catA.setName("CAT-A");
        m_manager->saveCategory(&catA);

        QLandmarkCategory catB;
        catB.setName("CAT-B");
        m_manager->saveCategory(&catB);

        QLandmarkCategoryId catIdNotExist;
        catIdNotExist.setManagerUri(m_manager->managerUri());

        QLandmarkCategoryId catIdNotExist2;

        QList<QLandmarkCategoryId> catIds;
        QSignalSpy spyResult(&catFetchByIdRequest, SIGNAL(resultsAvailable()));
        catIds << catA.categoryId() << catIdNotExist << catB.categoryId() << catIdNotExist2;
        catFetchByIdRequest.setCategoryIds(catIds);
        catFetchByIdRequest.start();
        QVERIFY(waitForAsync(spy, &catFetchByIdRequest, QLandmarkManager::DoesNotExistError,100));
        cats = catFetchByIdRequest.categories();
        QCOMPARE(cats.count(), 4);
        QCOMPARE(cats.at(0), catA);
        QCOMPARE(cats.at(1), QLandmarkCategory());
        QCOMPARE(cats.at(2), catB);
        QCOMPARE(cats.at(3), QLandmarkCategory());
        QCOMPARE(catFetchByIdRequest.errorMap().count(), 2);
        QCOMPARE(catFetchByIdRequest.errorMap().keys().at(0), 1);
        QCOMPARE(catFetchByIdRequest.errorMap().keys().at(1), 3);
        QCOMPARE(catFetchByIdRequest.errorMap().value(1), QLandmarkManager::DoesNotExistError);
        QCOMPARE(catFetchByIdRequest.errorMap().value(3), QLandmarkManager::DoesNotExistError);
        QCOMPARE(spyResult.count(), 1);
        spyResult.clear();

        catIds.clear();
        catIds << catA.categoryId() << catB.categoryId() << cat2.categoryId();
        catFetchByIdRequest.setCategoryIds(catIds);
        catFetchByIdRequest.start();
        QVERIFY(waitForAsync(spy, &catFetchByIdRequest, QLandmarkManager::NoError,100));
        cats = catFetchByIdRequest.categories();
        QCOMPARE(cats.count(), 3);
        QCOMPARE(cats.at(0), catA);
        QCOMPARE(cats.at(1), catB);
        QCOMPARE(cats.at(2), cat2);
        QCOMPARE(catFetchByIdRequest.errorMap().count(), 0);
        QCOMPARE(spyResult.count(),1);
        spyResult.clear();
    }
}

void tst_QLandmarkManager::retrieveCategory_data() {
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

#ifdef RESTORE_CANCEL_CATEGORY_FETCH
void tst_QLandmarkManager::categoryFetchCancelAsync()
{
    disconnectNotifications();
    QLandmarkCategory cat;
    for(int i=0; i < 550; ++i) {
        cat.clear();
        cat.setName(QString("CAT") + QString::number(i));
        QVERIFY(m_manager->saveCategory(&cat));
    }

    connectNotifications();
    QLandmarkCategoryFetchRequest catFetchRequest(m_manager);
    QSignalSpy spy(&catFetchRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
    catFetchRequest.start();
#ifdef Q_OS_SYMBIAN
#else
    QTest::qWait(50);
#endif
    catFetchRequest.cancel();
    QVERIFY(waitForAsync(spy, &catFetchRequest, QLandmarkManager::CancelError,2000));
    QCOMPARE(catFetchRequest.categories().count(), 0);

    /*
    Testing cancel for category id fetch is difficult because on
            desktop, the operation is too quick to cancel.  We could
            try populating the database with enough categories but
            that would take a very long time.

            for(int i=0; i < 350; ++i) {
        cat.clear();
        cat.setName(QString("CAT") + QString::number(i));
        QVERIFY(m_manager->saveCategory(&cat));
    }

    QLandmarkCategoryIdFetchRequest catIdFetchRequest(m_manager);
    QSignalSpy spy2(&catIdFetchRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
    catIdFetchRequest.start();
    QTest::qWait(10);
    catIdFetchRequest.cancel();
    QVERIFY(waitForAsync(spy2, &catIdFetchRequest, QLandmarkManager::CancelError));
    */
}
#endif

void tst_QLandmarkManager::retrieveLandmark() {
    QFETCH(QString, type);

    //pre-populate the manager to make sure we don't accidentally retrieve these
    //landmarks
    QLandmark lmAlpha;
    lmAlpha.setName("LM-ALPHA");
    QVERIFY(m_manager->saveLandmark(&lmAlpha));

    QLandmark lmBeta;
    lmBeta.setName("LM-BETA");
    QVERIFY(m_manager->saveLandmark(&lmBeta));

    QLandmark lmGamma;
    lmGamma.setName("LM-GAMMA");
    QVERIFY(m_manager->saveLandmark(&lmGamma));

    QLandmarkFetchByIdRequest lmFetchByIdRequest(m_manager);
    QSignalSpy spy(&lmFetchByIdRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
    QList<QLandmark> lms;

    QLandmarkId id1;
    if (type == "sync") {
        //try a default constructed landmark id
        QCOMPARE(m_manager->landmark(QLandmarkId()), QLandmark());
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(m_manager->landmark(QLandmarkId()).landmarkId().isValid(),false);

        //try an id that does not exist
        id1.setManagerUri(m_manager->managerUri());
        id1.setLocalId("100");
        QCOMPARE(m_manager->landmark(id1), QLandmark());
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(m_manager->landmark(id1).landmarkId().isValid(), false);
    } else if (type == "async") {
        //try a default constructed landmark id
        lmFetchByIdRequest.setLandmarkId(QLandmarkId());
        lmFetchByIdRequest.start();
        QVERIFY(waitForAsync(spy, &lmFetchByIdRequest,QLandmarkManager::DoesNotExistError,100));
        QCOMPARE(lmFetchByIdRequest.errorMap().count(),1);
        QCOMPARE(lmFetchByIdRequest.errorMap().keys().at(0),0);
        QCOMPARE(lmFetchByIdRequest.errorMap().value(0), QLandmarkManager::DoesNotExistError);

        lms = lmFetchByIdRequest.landmarks();
        QCOMPARE(lms.count(), 1);
        QCOMPARE(lms.at(0), QLandmark());
        QCOMPARE(lms.at(0).landmarkId().isValid(), false);

        //try an id that does not exist
        id1.setManagerUri(m_manager->managerUri());
        id1.setLocalId("100");
        lmFetchByIdRequest.setLandmarkId(id1);
        lmFetchByIdRequest.start();
        QVERIFY(waitForAsync(spy, &lmFetchByIdRequest,QLandmarkManager::DoesNotExistError,100));
        QCOMPARE(lmFetchByIdRequest.errorMap().count(),1);
        QCOMPARE(lmFetchByIdRequest.errorMap().keys().at(0),0);
        QCOMPARE(lmFetchByIdRequest.errorMap().value(0), QLandmarkManager::DoesNotExistError);

        lms = lmFetchByIdRequest.landmarks();
        QCOMPARE(lms.count(), 1);
        QCOMPARE(lms.at(0), QLandmark());
        QCOMPARE(lms.at(0).landmarkId().isValid(), false);
    } else {
        qFatal("Unknown test row type");
    }

    QLandmark lm2;
    lm2.setName("LM2");
    QGeoAddress address;
    address.setStreet("LM2 street");
    address.setDistrict("LM2 district");
    address.setCity("LM2 city");
    address.setState("LM2 State");
    address.setCountry("LM2 Country");
    address.setCountryCode("LM2CountryCode");
    address.setPostCode("LM2 post code");
    lm2.setAddress(address);
    QGeoCoordinate coordinate(10,20);
    lm2.setCoordinate(coordinate);
    lm2.setPhoneNumber("2nd phone number");
    lm2.setIconUrl(QUrl("2nd iconUrl"));
    lm2.setRadius(2000);
    lm2.setUrl(QUrl("2nd url"));
    QVERIFY(m_manager->saveLandmark(&lm2));

    QLandmarkId id2;
    id2.setManagerUri(m_manager->managerUri());
    id2.setLocalId(lm2.landmarkId().localId());
    if (type == "sync") {
        id2.setManagerUri("wrong.manager");
        id2.setLocalId(lm2.landmarkId().localId());
        QCOMPARE(m_manager->landmark(id2), QLandmark());
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(m_manager->landmark(id2).landmarkId().isValid(), false);

        id2 = lm2.landmarkId();
        QCOMPARE(m_manager->landmark(id2), lm2);
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
        QCOMPARE(m_manager->landmark(id2).landmarkId().isValid(), true);

        //ensure consecutive calls clears the error
        QCOMPARE(m_manager->landmark(id1), QLandmark());
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(m_manager->landmark(id2), lm2);
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    } else if (type == "async") {
        //try a manager that doesn't match
        id2.setManagerUri("wrong.manager");
        id2.setLocalId(lm2.landmarkId().localId());
        lmFetchByIdRequest.setLandmarkId(id2);
        lmFetchByIdRequest.start();
        QVERIFY(waitForAsync(spy, &lmFetchByIdRequest, QLandmarkManager::DoesNotExistError,100));
        QCOMPARE(lmFetchByIdRequest.errorMap().count(),1);
        QCOMPARE(lmFetchByIdRequest.errorMap().keys().at(0),0);
        QCOMPARE(lmFetchByIdRequest.errorMap().value(0), QLandmarkManager::DoesNotExistError);

        lms = lmFetchByIdRequest.landmarks();
        QCOMPARE(lms.count(), 1);
        QCOMPARE(lms.at(0), QLandmark());
        QCOMPARE(lms.at(0).landmarkId().isValid(), false);

        //try an existing id
        id2 = lm2.landmarkId();
        lmFetchByIdRequest.setLandmarkId(id2);
        lmFetchByIdRequest.start();
        QVERIFY(waitForAsync(spy, &lmFetchByIdRequest, QLandmarkManager::NoError,100));
        QCOMPARE(lmFetchByIdRequest.errorMap().count(),0);

        lms = lmFetchByIdRequest.landmarks();
        QCOMPARE(lms.count(), 1);
        QCOMPARE(lms.at(0), lm2);
        QCOMPARE(lms.at(0).landmarkId().isValid(), true);
    } else {
        qFatal("Unkown test row type");
    }

    //try retrieving a landmark with a category
    QLandmarkCategory cat1;
    cat1.setName("CAT1");
    QVERIFY(m_manager->saveCategory(&cat1));

    QLandmarkCategory cat2;
    cat2.setName("CAT2");
    QVERIFY(m_manager->saveCategory(&cat2));

    QLandmarkCategory cat3;
    cat3.setName("CAT3");
    QVERIFY(m_manager->saveCategory(&cat3));

    QLandmark lm3;
    lm3.setName("LM3");
    lm3.addCategoryId(cat1.categoryId());
    QVERIFY(m_manager->saveLandmark(&lm3));

    QLandmark lm4;
    lm4.setName("LM4");
    lm4.addCategoryId(cat3.categoryId());
    lm4.addCategoryId(cat2.categoryId());
    QVERIFY(m_manager->saveLandmark(&lm4));

    QLandmarkId id3;
    id3 = lm3.landmarkId();

    QLandmarkId id4;
    id4 = lm4.landmarkId();

    if (type == "sync") {
        //check that we can retrieve a landmark a single catergory
        QLandmark lm3Retrieved = m_manager->landmark(id3);
        QCOMPARE(lm3Retrieved, lm3);
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
        QList<QLandmarkCategoryId> lm3RetrievedCatIds;
        lm3RetrievedCatIds = lm3Retrieved.categoryIds();
        QCOMPARE(lm3RetrievedCatIds.count(),1);
        QVERIFY(lm3RetrievedCatIds.contains(cat1.categoryId()));

        //check that we can retrieve a landmark with multiple categories
        QLandmark lm4Retrieved = m_manager->landmark(id4);
        QCOMPARE(lm4Retrieved, lm4);
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);

        QList<QLandmarkCategoryId> lm4RetrievedCatIds;
        lm4RetrievedCatIds = lm4Retrieved.categoryIds();
        QCOMPARE(lm4RetrievedCatIds.count(),2);
        QVERIFY(lm4RetrievedCatIds.contains(cat2.categoryId()));
        QVERIFY(lm4RetrievedCatIds.contains(cat3.categoryId()));
    } else if (type == "async") {
        //check that we can retrieve a landmark with a single catergory
        lmFetchByIdRequest.setLandmarkId(id3);
        lmFetchByIdRequest.start();
        QVERIFY(waitForAsync(spy, &lmFetchByIdRequest, QLandmarkManager::NoError,100));
        QCOMPARE(lmFetchByIdRequest.errorMap().count(),0);

        lms = lmFetchByIdRequest.landmarks();
        QCOMPARE(lms.count(), 1);
        QLandmark lm3Retrieved = lms.at(0);
        QCOMPARE(lm3Retrieved, lm3);
        QList<QLandmarkCategoryId> lm3RetrievedCatIds = lms.at(0).categoryIds();
        QCOMPARE(lm3RetrievedCatIds.count(),1);
        QVERIFY(lm3RetrievedCatIds.contains(cat1.categoryId()));

        //check that we can retrieve a landmark with multiple categories
        lmFetchByIdRequest.setLandmarkId(id4);
        lmFetchByIdRequest.start();
        QVERIFY(waitForAsync(spy, &lmFetchByIdRequest, QLandmarkManager::NoError,100));
        QCOMPARE(lmFetchByIdRequest.errorMap().count(),0);

        lms = lmFetchByIdRequest.landmarks();
        QCOMPARE(lms.count(), 1);
        QLandmark lm4Retrieved = lms.at(0);
        QCOMPARE(lm4Retrieved, lm4);

        QList<QLandmarkCategoryId> lm4RetrievedCatIds = lms.at(0).categoryIds();
        QCOMPARE(lm4RetrievedCatIds.count(),2);
        QVERIFY(lm4RetrievedCatIds.contains(cat2.categoryId()));
        QVERIFY(lm4RetrievedCatIds.contains(cat3.categoryId()));
    } else {
        qFatal("Unknown test row type");
    }

    //Test retrieval of multiple landmarks by id
    QLandmark lmA;
    lmA.setName("LMA");
    address.clear();
    address.setStreet("LMA street");
    address.setDistrict("LMA district");
    address.setCity("LMA city");
    address.setState("LMA State");
    address.setCountry("LMA Country");
    address.setCountryCode("LMACountryCode");
    address.setPostCode("LMA post code");
    lmA.setAddress(address);
    coordinate.setLatitude(50);
    coordinate.setLongitude(24);
    lmA.setCoordinate(coordinate);
    lmA.setPhoneNumber("phone number A");
    lmA.setIconUrl(QUrl("iconUrl A"));
    lmA.setRadius(2000);
    lmA.setUrl(QUrl("url A"));
    lmA.addCategoryId(cat3.categoryId());

    QLandmark lmB;
    lmB.setName("LMB");
    address.clear();
    address.setStreet("LMB street");
    address.setDistrict("LMB district");
    address.setCity("LMB city");
    address.setState("LMB State");
    address.setCountry("LMB Country");
    address.setCountryCode("LMBCountryCode");
    address.setPostCode("LMB post code");
    lmB.setAddress(address);
    coordinate.setLatitude(-43);
    coordinate.setLongitude(-10);
    lmB.setCoordinate(coordinate);
    lmB.setPhoneNumber("phone number B");
    lmB.setIconUrl(QUrl("iconUrl B"));
    lmB.setRadius(2000);
    lmB.setUrl(QUrl("url B"));
    lmB.addCategoryId(cat1.categoryId());
    lmB.addCategoryId(cat2.categoryId());

    lms.clear();
    lms << lmA << lmB;
    QVERIFY(m_manager->saveLandmarks(&lms));

    QList<QLandmarkId> lmIds;
    QLandmarkId lmIdNotExist;
    lmIdNotExist.setManagerUri(m_manager->managerUri());
    lmIdNotExist.setLocalId("5000");

    QLandmarkId lmIdNotExist2;

    lmIds.clear();
    lmIds <<  lms.at(0).landmarkId() << lmIdNotExist << lms.at(1).landmarkId()
          << lmIdNotExist2;
    lmA.setLandmarkId(lms.at(0).landmarkId());
    lmB.setLandmarkId(lms.at(1).landmarkId());

    if (type == "sync") {
        //fetch landmarks without an errormap
        lms = m_manager->landmarks(lmIds);
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(lms.count(),4);
        QCOMPARE(lms.at(0), lmA);
        QCOMPARE(lms.at(1), QLandmark());
        QCOMPARE(lms.at(2), lmB);
        QCOMPARE(lms.at(3), QLandmark());

        //fetch landmarks wtih an error map
        QMap <int, QLandmarkManager::Error> errorMap;
       lms = m_manager->landmarks(lmIds, &errorMap);
       QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
       QCOMPARE(lms.count(),4);
       QCOMPARE(lms.at(0), lmA);
       QCOMPARE(lms.at(1), QLandmark());
       QCOMPARE(lms.at(2), lmB);
       QCOMPARE(lms.at(3), QLandmark());

       QCOMPARE(errorMap.count(), 2);
       QCOMPARE(errorMap.keys().at(0),1);
       QCOMPARE(errorMap.keys().at(1),3);
       QCOMPARE(errorMap.value(1), QLandmarkManager::DoesNotExistError);
       QCOMPARE(errorMap.value(3), QLandmarkManager::DoesNotExistError);

       //check that the error map will be cleared
       lmIds.clear();
       lmIds << lmA.landmarkId() << lmB.landmarkId() << lm4.landmarkId();
       lms = m_manager->landmarks(lmIds, &errorMap);
       QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
       QCOMPARE(lms.count(), 3);
       QCOMPARE(lms.at(0),lmA);
       QCOMPARE(lms.at(1), lmB);
       QCOMPARE(lms.at(2), lm4);
       QCOMPARE(errorMap.count(), 0);

   } else if (type == "async") {
       QLandmarkFetchByIdRequest fetchByIdRequest(m_manager);
       QSignalSpy spy(&fetchByIdRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
       QSignalSpy spyResult(&fetchByIdRequest, SIGNAL(resultsAvailable()));
       fetchByIdRequest.setLandmarkIds(lmIds);
       fetchByIdRequest.start();
       QVERIFY(waitForAsync(spy, &fetchByIdRequest,QLandmarkManager::DoesNotExistError,1000));
       QCOMPARE(fetchByIdRequest.errorMap().count(),2);
       QCOMPARE(fetchByIdRequest.errorMap().keys().at(0),1);
       QCOMPARE(fetchByIdRequest.errorMap().keys().at(1),3);
       QCOMPARE(fetchByIdRequest.errorMap().value(1), QLandmarkManager::DoesNotExistError);
       QCOMPARE(fetchByIdRequest.errorMap().value(3), QLandmarkManager::DoesNotExistError);

       QCOMPARE(spyResult.count(), 1);
       spyResult.clear();

       lms = fetchByIdRequest.landmarks();
       QCOMPARE(lms.count(),4);
       QCOMPARE(lms.at(0), lmA);
       QCOMPARE(lms.at(1), QLandmark());
       QCOMPARE(lms.at(2), lmB);
       QCOMPARE(lms.at(3), QLandmark());

       //check that the error map will be cleared
       lmIds.clear();
       lmIds << lmA.landmarkId() << lmB.landmarkId() << lm4.landmarkId();
       fetchByIdRequest.setLandmarkIds(lmIds);
       fetchByIdRequest.start();

       QVERIFY(waitForAsync(spy, &fetchByIdRequest,QLandmarkManager::NoError,1000));
       lms = fetchByIdRequest.landmarks();
       QCOMPARE(lms.count(), 3);
       QCOMPARE(lms.at(0),lmA);
       QCOMPARE(lms.at(1), lmB);
       QCOMPARE(lms.at(2), lm4);
       QCOMPARE(fetchByIdRequest.errorMap().count(),0);
       QCOMPARE(spyResult.count(), 1);
       spyResult.clear();
   }
}

void tst_QLandmarkManager::retrieveLandmark_data() {
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::saveCategory() {
    QFETCH(QString, type);
    QSignalSpy spyAdd(m_manager, SIGNAL(categoriesAdded(QList<QLandmarkCategoryId>)));
    QSignalSpy spyChange(m_manager,SIGNAL(categoriesChanged(QList<QLandmarkCategoryId>)));
    QSignalSpy spyRemove(m_manager,SIGNAL(categoriesRemoved(QList<QLandmarkCategoryId>)));

    int originalCategoryCount = m_manager->categoryIds().count();
    QCOMPARE(originalCategoryCount, m_manager->categories().count());

    //try save an empty category name
    QLandmarkCategory emptyCategory;
    QVERIFY(doSingleCategorySave(type,&emptyCategory,QLandmarkManager::BadArgumentError));
    QVERIFY(checkCategoryCount(originalCategoryCount));

    QLandmarkCategory cat1;
    cat1.setName("CAT1");
    cat1.setIconUrl(QUrl("CAT1Url"));

    QLandmarkCategory cat1Initial = cat1;
    QVERIFY(doSingleCategorySave(type,&cat1,QLandmarkManager::NoError));
    cat1Initial.setCategoryId(cat1.categoryId());
    QCOMPARE(cat1, cat1Initial);

    QTest::qWait(10);
    QCOMPARE(spyAdd.count(), 1);
    QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(0), cat1.categoryId());
    QCOMPARE(spyChange.count(), 0);
    QCOMPARE(spyRemove.count(), 0);
    spyAdd.clear();
    QVERIFY(checkCategoryCount(originalCategoryCount+1));

    QLandmarkCategory cat2;
    cat2.setName("CAT2");
    QVERIFY(doSingleCategorySave(type, &cat2, QLandmarkManager::NoError));

    QTest::qWait(10);
    QCOMPARE(spyAdd.count(), 1);
    QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(0), cat2.categoryId());
    QCOMPARE(spyChange.count(), 0);
    QCOMPARE(spyRemove.count(), 0);
    spyAdd.clear();

    QVERIFY(checkCategoryCount(originalCategoryCount + 2));
    int newCategoryCount = originalCategoryCount + 2;

   //try adding a category when the category name already exists
   QLandmarkCategory cat1Duplicate;
   cat1Duplicate.setName("CAT1");
   QVERIFY(doSingleCategorySave(type, &cat1Duplicate,QLandmarkManager::AlreadyExistsError));
   QCOMPARE(m_manager->categories().count(), newCategoryCount);
   QVERIFY(checkCategoryCount(newCategoryCount));

   //try renaming an existing category to a name that already exists
   QLandmarkCategory cat2Original = cat2;
   QLandmarkCategoryId cat2OriginalId = cat2.categoryId();
   cat2.setName("CAT1");
   QVERIFY(doSingleCategorySave(type, &cat2,QLandmarkManager::AlreadyExistsError));
   QVERIFY(checkCategoryCount(newCategoryCount));
   QCOMPARE(m_manager->category(cat2OriginalId).name(), QString("CAT2"));

   //try save a category whose manager uri does not match
   cat2.setName("CAT2next");
   QLandmarkCategoryId id;
   id = cat2.categoryId();
   id.setManagerUri("wrong.manager");
   cat2.setCategoryId(id);
   QVERIFY(doSingleCategorySave(type, &cat2, QLandmarkManager::DoesNotExistError));
   QCOMPARE(m_manager->category(cat2OriginalId).name(), QString("CAT2"));
   QVERIFY(checkCategoryCount(newCategoryCount));

   QLandmarkCategory accommodation;
   QList<QLandmarkCategory> cats = m_manager->categories();
   for (int i=0; i < cats.count(); ++i) {
       if (cats.at(i).name() == "Accommodation")
           accommodation = cats.at(i);
   }

#ifdef Q_OS_SYMBIAN
   //check that global category is read-only
   QVERIFY(m_manager->isReadOnly(accommodation.categoryId()));

#ifdef UNRESOLVED_SAVING_GLOBAL_CATEGORY
   //try save a global category
   m_manager->saveCategory(&accommodation);
   QVERIFY(doSingleCategorySave(type, &accommodation, QLandmarkManager::PermissionsError));

   //try rename a global category;
   accommodation.setName("hotels");
   m_manager->saveCategory(&accommodation);
   QVERIFY(doSingleCategorySave(type, &accommodation, QLandmarkManager::PermissionsError));
#endif

   //try save a (new) category with the same name as a global category
   QLandmarkCategory accommodationDuplicate;
   accommodationDuplicate.setName("Accommodation");
   QVERIFY(doSingleCategorySave(type, &accommodationDuplicate, QLandmarkManager::AlreadyExistsError));


#endif

   //try modify a category that does not exist
   QLandmarkCategory catDoesNotExist = cat2Original;
   catDoesNotExist.setName("DoesNotExist");
   QLandmarkCategoryId catDoesNotExistId = cat2Original.categoryId();
   catDoesNotExistId.setLocalId("10000");
   catDoesNotExist.setCategoryId(catDoesNotExistId);
   QVERIFY(doSingleCategorySave(type, &catDoesNotExist,QLandmarkManager::DoesNotExistError));
   QVERIFY(checkCategoryCount(newCategoryCount));

   QTest::qWait(10);
   QCOMPARE(spyAdd.count(), 0);
   QCOMPARE(spyChange.count(), 0);
   QCOMPARE(spyRemove.count(), 0);

   //try modify a category that does already exist
   QLandmarkCategory cat2Modified = cat2Original;
   cat2Modified.setName("CAT2Modified");
   cat2Modified.setIconUrl(QUrl("cat2 url modified"));
   QVERIFY(doSingleCategorySave(type,&cat2Modified,QLandmarkManager::NoError));
   QVERIFY(checkCategoryCount(newCategoryCount));
   QCOMPARE(m_manager->category(cat2OriginalId).name(), QString("CAT2Modified"));
   QCOMPARE(m_manager->category(cat2OriginalId).iconUrl(), QUrl("cat2 url modified"));

   QTest::qWait(10);
   QCOMPARE(spyAdd.count(), 0);
   QCOMPARE(spyChange.count(), 1);
   QCOMPARE(spyRemove.count(), 0);
   spyChange.clear();

   if (type == "async"){
       //save multiple categories where the
       //first is a new category, second should fail, third is an update
       QLandmarkCategory catNew;
       catNew.setName("CATNew");

       QLandmarkCategory catBad;
       catBad.setName("CATBad");
       QLandmarkCategoryId catBadId;
       catBadId = cat2Modified.categoryId();
       catBadId.setLocalId("1000");
       catBad.setCategoryId(catBadId);

       QLandmarkCategory catChange = cat2Modified;
       catChange.setName("CATChange");

       QList<QLandmarkCategory> cats;
       cats << catNew << catBad << catChange;
       QLandmarkCategorySaveRequest saveCategoryRequest(m_manager);
       QSignalSpy spy(&saveCategoryRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
       saveCategoryRequest.setCategories(cats);
       saveCategoryRequest.start();
       QVERIFY(waitForAsync(spy, &saveCategoryRequest, QLandmarkManager::DoesNotExistError));
       QCOMPARE(saveCategoryRequest.categories().count(), 3);
       QCOMPARE(saveCategoryRequest.errorMap().count(),1);
       QCOMPARE(saveCategoryRequest.errorMap().value(1) , QLandmarkManager::DoesNotExistError);

       QLandmarkCategoryId firstCategoryId = saveCategoryRequest.categories().at(0).categoryId();
       QLandmarkCategoryId secondCategoryId = saveCategoryRequest.categories().at(1).categoryId();
       QLandmarkCategoryId thirdCategoryId = saveCategoryRequest.categories().at(2).categoryId();

       //check that the new and changed categories will match the originals
       //(which wouldn't have category ids so we just assign those now)
       catNew.setCategoryId(firstCategoryId);
       catChange.setCategoryId(thirdCategoryId);

       QCOMPARE(catNew, m_manager->category(firstCategoryId));
       QVERIFY(!m_manager->category(secondCategoryId).categoryId().isValid());
       QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
       QCOMPARE(catChange, m_manager->category(thirdCategoryId));

       QCOMPARE(catNew, saveCategoryRequest.categories().at(0));
       QCOMPARE(catBad, saveCategoryRequest.categories().at(1));
       QCOMPARE(catChange, saveCategoryRequest.categories().at(2));

       QTest::qWait(10);
       QCOMPARE(spyAdd.count(), 1);
       QCOMPARE(spyChange.count(), 1);
       QCOMPARE(spyRemove.count(), 0);
       QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(0), firstCategoryId);
       QCOMPARE(spyChange.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(0), thirdCategoryId);
       spyAdd.clear();
       spyChange.clear();

       //try saving multiple new categories
       QLandmarkCategory catNew1;
       catNew1.setName("CATNew1");
       catNew1.setIconUrl(QUrl("CATNew1 iconUrl"));

       QLandmarkCategory catNew2;
       catNew2.setName("CATNew2");
       catNew2.setIconUrl(QUrl("CATNew2 iconUrl"));

       QLandmarkCategory catNew3;
       catNew3.setName("CATNew3");
       catNew3.setIconUrl(QUrl("CATNew3 iconUrl"));
       cats.clear();
       cats << catNew1 << catNew2 << catNew3;
       saveCategoryRequest.setCategories(cats);
       saveCategoryRequest.start();
       QVERIFY(waitForAsync(spy, &saveCategoryRequest, QLandmarkManager::NoError));
       QCOMPARE(saveCategoryRequest.categories().count(), 3);
       QCOMPARE(saveCategoryRequest.errorMap().count(),0);

       firstCategoryId = saveCategoryRequest.categories().at(0).categoryId();
       secondCategoryId = saveCategoryRequest.categories().at(1).categoryId();
       thirdCategoryId = saveCategoryRequest.categories().at(2).categoryId();

       catNew1.setCategoryId(firstCategoryId);
       catNew2.setCategoryId(secondCategoryId);
       catNew3.setCategoryId(thirdCategoryId);
       QCOMPARE(catNew1, m_manager->category(firstCategoryId));
       QCOMPARE(catNew2, m_manager->category(secondCategoryId));
       QCOMPARE(catNew3, m_manager->category(thirdCategoryId));

       QCOMPARE(catNew1, saveCategoryRequest.categories().at(0));
       QCOMPARE(catNew2, saveCategoryRequest.categories().at(1));
       QCOMPARE(catNew3, saveCategoryRequest.categories().at(2));

       QTest::qWait(10);
       QCOMPARE(spyAdd.count(), 1);
       QCOMPARE(spyChange.count(), 0);
       QCOMPARE(spyRemove.count(), 0);
       QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(0), firstCategoryId);
       QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(1), secondCategoryId);
       QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(2), thirdCategoryId);
       spyAdd.clear();

       //try changing multiple categories
       catNew1.setName("CATNew1Changed");
       catNew2.setIconUrl(QUrl("CATNew2 iconUrlChanged"));
       catNew3.setName("CATNew3Changed");
       catNew3.setIconUrl(QUrl("catNew3Changed"));
       cats.clear();
       cats << catNew1 << catNew2 << catNew3;
       saveCategoryRequest.setCategories(cats);
       saveCategoryRequest.start();
       QVERIFY(waitForAsync(spy, &saveCategoryRequest, QLandmarkManager::NoError));
       QCOMPARE(saveCategoryRequest.categories().count(), 3);
       QCOMPARE(saveCategoryRequest.errorMap().count(),0);

       firstCategoryId = saveCategoryRequest.categories().at(0).categoryId();
       secondCategoryId = saveCategoryRequest.categories().at(1).categoryId();
       thirdCategoryId = saveCategoryRequest.categories().at(2).categoryId();

       QCOMPARE(catNew1, m_manager->category(firstCategoryId));
       QCOMPARE(catNew2, m_manager->category(secondCategoryId));
       QCOMPARE(catNew3, m_manager->category(thirdCategoryId));

       QCOMPARE(catNew1, saveCategoryRequest.categories().at(0));
       QCOMPARE(catNew2, saveCategoryRequest.categories().at(1));
       QCOMPARE(catNew3, saveCategoryRequest.categories().at(2));

       QTest::qWait(10);
       QCOMPARE(spyAdd.count(), 0);
       QCOMPARE(spyChange.count(), 1);
       QCOMPARE(spyRemove.count(), 0);
       QCOMPARE(spyChange.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(0), firstCategoryId);
       QCOMPARE(spyChange.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(1), secondCategoryId);
       QCOMPARE(spyChange.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(2), thirdCategoryId);
       spyChange.clear();
   }

   //chec that error is cleared on consecutive saves
   if (type == "sync") {
       QLandmarkCategory catAlpha;
       QVERIFY(!m_manager->saveCategory(&catAlpha));
       QCOMPARE(m_manager->error(), QLandmarkManager::BadArgumentError);
       catAlpha.setName("CAT-Alpha");
       QVERIFY(m_manager->saveCategory(&catAlpha));
       QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
   }
}

void tst_QLandmarkManager::saveCategory_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::saveLandmark() {
    QFETCH(QString, type);
    QSignalSpy spyAdd(m_manager, SIGNAL(landmarksAdded(QList<QLandmarkId>)));
    QSignalSpy spyChange(m_manager, SIGNAL(landmarksChanged(QList<QLandmarkId>)));
    QSignalSpy spyRemove(m_manager, SIGNAL(landmarksRemoved(QList<QLandmarkId>)));

     int originalLandmarkCount = m_manager->landmarks().count();
    QLandmark emptyLandmark;
    QVERIFY(doSingleLandmarkSave(type, &emptyLandmark,QLandmarkManager::NoError));
    QVERIFY(checkLandmarkCount(originalLandmarkCount + 1));

    QTest::qWait(10);
    QCOMPARE(spyAdd.count(), 1);
    QCOMPARE(spyChange.count(), 0);
    QCOMPARE(spyRemove.count(),0);
    QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkId> >().at(0), emptyLandmark.landmarkId());
    spyAdd.clear();

    QLandmarkCategory cat1;
    cat1.setName("CAT1");
    QVERIFY(m_manager->saveCategory(&cat1));

    QLandmarkCategory cat2;
    cat2.setName("CAT2");
    QVERIFY(m_manager->saveCategory(&cat2));

    QLandmarkCategory cat3;
    cat3.setName("cat3");
    QVERIFY(m_manager->saveCategory(&cat3));

    //save a new landamrk
    QLandmark lm1;
    lm1.setName("LM1");
    QGeoAddress address;
    address.setStreet("LM1 street");
    address.setDistrict("LM1 district");
    address.setCity("LM1 city");
    address.setState("LM1 State");
    address.setCountry("LM1 Country");
    address.setCountryCode("LM1CountryCode");
    address.setPostCode("LM1 post code");
    lm1.setAddress(address);
    QGeoCoordinate coordinate(10,20);
    lm1.setCoordinate(coordinate);
    lm1.setPhoneNumber("LM1 phone number");
    lm1.setIconUrl(QUrl("LM1 iconUrl"));
    lm1.setRadius(1000);
    lm1.setUrl(QUrl("LM1 url"));
    lm1.addCategoryId(cat2.categoryId());
    lm1.addCategoryId(cat3.categoryId());
    QLandmark lm1Initial = lm1;

    QVERIFY(doSingleLandmarkSave(type, &lm1,QLandmarkManager::NoError));
    QVERIFY(checkLandmarkCount(originalLandmarkCount +2));
    int newLandmarkCount = originalLandmarkCount + 2;

    QCOMPARE(lm1, m_manager->landmark(lm1.landmarkId()));
    lm1Initial.setLandmarkId(lm1.landmarkId());
    QCOMPARE(lm1,lm1Initial);

    QTest::qWait(10);
    QCOMPARE(spyAdd.count(), 1);
    QCOMPARE(spyChange.count(), 0);
    QCOMPARE(spyRemove.count(),0);
    QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkId> >().at(0), lm1.landmarkId());
    spyAdd.clear();

    //update a landmark
    QLandmark lm1Changed = lm1;
    lm1Changed.setName("LM1Changed");
    address.setStreet("LM1Changed street");
    address.setDistrict("LM1Changed district");
    address.setCity("LM1Changed city");
    address.setState("LM1Changed State");
    address.setCountry("LM1Changed Country");
    address.setCountryCode("LM1Changed CountryCode");
    address.setPostCode("LM1Changed Post code");
    lm1Changed.setAddress(address);
    coordinate.setLatitude(11);
    coordinate.setLongitude(21);
    lm1Changed.setCoordinate(coordinate);
    lm1Changed.setPhoneNumber("LM1Changed phone number");
    lm1Changed.setIconUrl(QUrl("LM1Changed iconUrl"));
    lm1Changed.setRadius(1100);
    lm1Changed.setUrl(QUrl("LM1Changed url"));
    lm1Changed.removeCategoryId(cat3.categoryId());
    lm1Changed.removeCategoryId(cat2.categoryId());
    lm1Changed.addCategoryId(cat1.categoryId());
    QLandmark lm1ChangedInitial = lm1Changed;

    QVERIFY(doSingleLandmarkSave(type, &lm1Changed, QLandmarkManager::NoError));
    QVERIFY(checkLandmarkCount(newLandmarkCount));
    QCOMPARE(lm1ChangedInitial, m_manager->landmark(lm1.landmarkId()));
    QCOMPARE(lm1ChangedInitial, lm1Changed);

    QTest::qWait(10);
    QCOMPARE(spyAdd.count(), 0);
    QCOMPARE(spyChange.count(), 1);
    QCOMPARE(spyRemove.count(),0);
    QCOMPARE(spyChange.at(0).at(0).value<QList<QLandmarkId> >().at(0), lm1Changed.landmarkId());
    spyChange.clear();

    //try clearing one of the fields
    lm1Changed.setPhoneNumber("");
    QVERIFY(doSingleLandmarkSave(type, &lm1Changed, QLandmarkManager::NoError));
    QVERIFY(checkLandmarkCount(newLandmarkCount));
    QCOMPARE(m_manager->landmark(lm1Changed.landmarkId()).phoneNumber(),QString());

    QTest::qWait(10);
    QCOMPARE(spyAdd.count(), 0);
    QCOMPARE(spyChange.count(), 1);
    QCOMPARE(spyRemove.count(),0);
    QCOMPARE(spyChange.at(0).at(0).value<QList<QLandmarkId> >().at(0), lm1Changed.landmarkId());
    spyChange.clear();

    //try saving a landmark with an invalid coordinate
    //i.e. one of latitude/longitude is Nan and the other is a valid number
    QLandmark lmInvalidCoordinate;
    lmInvalidCoordinate.setName("LMInvalid");
    QGeoCoordinate invalidCoordinate;
    invalidCoordinate.setLatitude(20);
    lmInvalidCoordinate.setCoordinate(invalidCoordinate);
    QLandmark lmInvalidCoordinateInitial = lmInvalidCoordinate;
    m_manager->saveLandmark(&lmInvalidCoordinate);

    QVERIFY(doSingleLandmarkSave(type, &lmInvalidCoordinate,QLandmarkManager::BadArgumentError));
    QVERIFY(checkLandmarkCount(newLandmarkCount));
    QCOMPARE(lmInvalidCoordinate, lmInvalidCoordinateInitial);

    //try saving a landmark where a coordinate is out of range(in constructor)
    QLandmark lmOutOfRange;
    lmOutOfRange.setName("lmOutOfRange");
    QGeoCoordinate outOfRangeCoord(91,45);
    //QGeoCoordinate should set its latitude and longitude values to NaN
    //meaning this is a 'valid' coordinate.
    QVERIFY(qIsNaN(outOfRangeCoord.latitude()));
    QVERIFY(qIsNaN(outOfRangeCoord.longitude()));
    lmOutOfRange.setCoordinate(outOfRangeCoord);
    QLandmark lmOutOfRangeInitial = lmOutOfRange;
    QVERIFY(doSingleLandmarkSave(type, &lmOutOfRange,QLandmarkManager::NoError));
    newLandmarkCount +=1;
    QVERIFY(checkLandmarkCount(newLandmarkCount));
    lmOutOfRangeInitial.setLandmarkId(lmOutOfRange.landmarkId());
    QCOMPARE(lmOutOfRange, lmOutOfRangeInitial);

    //try saving a landmark with an out of range latitude
    QLandmark lmOutOfRange2;
    outOfRangeCoord.setLatitude(91);
    outOfRangeCoord.setLongitude(45);
    lmOutOfRange2.setCoordinate(outOfRangeCoord);
    QLandmark lmOutOfRange2Initial = lmOutOfRange2;
    QVERIFY(doSingleLandmarkSave(type, &lmOutOfRange2,QLandmarkManager::BadArgumentError));
    QVERIFY(checkLandmarkCount(newLandmarkCount));
    QCOMPARE(lmOutOfRange2, lmOutOfRange2Initial);

    //try saving a landmark with an out of range longitude
    QLandmark lmOutOfRange3;
    outOfRangeCoord.setLatitude(-60);
    outOfRangeCoord.setLongitude(-180.1);
    lmOutOfRange3.setCoordinate(outOfRangeCoord);
    QLandmark lmOutOfRange3Initial = lmOutOfRange3;
    QVERIFY(doSingleLandmarkSave(type, &lmOutOfRange3,QLandmarkManager::BadArgumentError));
    QVERIFY(checkLandmarkCount(newLandmarkCount));
    QCOMPARE(lmOutOfRange3, lmOutOfRange3Initial);

    //try saving a landmark with a category that doesn't exist
    QLandmarkCategoryId catIdNotExist;
    catIdNotExist.setLocalId("1000");
    catIdNotExist.setManagerUri(m_manager->managerUri());
    QLandmark lmCatNotExist;
    lmCatNotExist.setName("LMCatNoExist");
    lmCatNotExist.addCategoryId(cat1.categoryId());
    lmCatNotExist.addCategoryId(catIdNotExist);
    QLandmark lmCatNotExistInitial = lmCatNotExist;
    QVERIFY(doSingleLandmarkSave(type, &lmCatNotExist,QLandmarkManager::BadArgumentError));
    QVERIFY(checkLandmarkCount(newLandmarkCount));
    QCOMPARE(lmCatNotExist, lmCatNotExistInitial);

    //try saving a landmark with a category belonging to a different manager
    QLandmarkCategoryId catIdManagerMismatch;
    catIdManagerMismatch.setLocalId(cat1.categoryId().localId());
    catIdManagerMismatch.setManagerUri("wrong.manager");
    QLandmark lmCatManagerMismatch;
    lmCatManagerMismatch.setName("LMCatManagerMismatch");
    lmCatManagerMismatch.addCategoryId(catIdManagerMismatch);
    lmCatManagerMismatch.addCategoryId(cat1.categoryId());
    QLandmark lmCatManagerMismatchInitial = lmCatManagerMismatch;
    m_manager->saveLandmark(&lmCatManagerMismatch);
    QVERIFY(doSingleLandmarkSave(type, &lmCatManagerMismatch, QLandmarkManager::BadArgumentError));
    QVERIFY(checkLandmarkCount(newLandmarkCount));
    QCOMPARE(lmCatManagerMismatch, lmCatManagerMismatchInitial);

    //try removing a category from a landmark
    QLandmark lm2;
    lm2.setName("LM2");
    lm2.addCategoryId(cat2.categoryId());
    QVERIFY(doSingleLandmarkSave(type, &lm2,QLandmarkManager::NoError));
    QVERIFY(m_manager->landmark(lm2.landmarkId()).categoryIds().contains(cat2.categoryId()));
    newLandmarkCount = newLandmarkCount +1;
    lm2.removeCategoryId(cat2.categoryId());
    QVERIFY(doSingleLandmarkSave(type, &lm2, QLandmarkManager::NoError));
    QCOMPARE(m_manager->landmark(lm2.landmarkId()).categoryIds().count(), 0);

    QTest::qWait(10);
    if (type == "sync")
        QCOMPARE(spyAdd.count(), 1);
    else if (type == "async")
        QCOMPARE(spyAdd.count(), 2);
    QCOMPARE(spyChange.count(), 1);
    QCOMPARE(spyRemove.count(),0);
    spyAdd.clear();
    spyChange.clear();

    //try saving a landmark which has an id but does not exist
    QLandmark lmNotExist = lm1Changed;
    QLandmarkId lmIdNotExist;
    lmIdNotExist.setLocalId("2000");
    lmIdNotExist.setManagerUri(m_manager->managerUri());
    lmNotExist.setLandmarkId(lmIdNotExist);
    QLandmark lmNotExistInitial = lmNotExist;
    QVERIFY(doSingleLandmarkSave(type, &lmNotExist, QLandmarkManager::DoesNotExistError));
    QVERIFY(checkLandmarkCount(newLandmarkCount));
    QCOMPARE(lmNotExist, lmNotExistInitial);

    QTest::qWait(10);
    QCOMPARE(spyAdd.count(), 0);
    QCOMPARE(spyChange.count(), 0);
    QCOMPARE(spyRemove.count(),0);

    //try saving multiple landamrks the first is new, the second should fail and the third is an update
    QLandmark lmNew;
    lmNew.setName("LMNew");
    address.clear();
    address.setStreet("LMNew Street");
    lmNew.setAddress(address);

    QLandmark lmBad;
    lmBad.setName("LMBad");
    address.clear();
    address.setStreet("LMBad Street");
    lmBad.setAddress(address);
    QLandmarkId lmBadId;
    lmBadId = lm1Changed.landmarkId();
    lmBadId.setLocalId("1000");
    lmBad.setLandmarkId(lmBadId);

    QLandmark lmChange = lm1Changed;
    lmChange.setName("LMChange");
    address.clear();
    address.setStreet("LMChange Street");
    lmChange.setAddress(address);

    QList<QLandmark> lms;
    lms << lmNew << lmBad << lmChange;
    QMap<int, QLandmarkManager::Error> errorMap;
    QVERIFY(doSave(type, &lms,QLandmarkManager::DoesNotExistError,&errorMap));
    QCOMPARE(lms.count(), 3);
    QCOMPARE(errorMap.count(), 1);
    QCOMPARE(errorMap.value(1), QLandmarkManager::DoesNotExistError);

    QLandmarkId firstLandmarkId = lms.at(0).landmarkId();
    QLandmarkId secondLandmarkId = lms.at(1).landmarkId();
    QLandmarkId thirdLandmarkId = lms.at(2).landmarkId();

    lmNew.setLandmarkId(firstLandmarkId);
    lmChange.setLandmarkId(thirdLandmarkId);

    QCOMPARE(lmNew, m_manager->landmark(firstLandmarkId));
    QCOMPARE(QLandmark(),m_manager->landmark(secondLandmarkId));
    QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
    QCOMPARE(lmChange, m_manager->landmark(thirdLandmarkId));

    QCOMPARE(lmNew, lms.at(0));
    QCOMPARE(lmBad, lms.at(1));
    QCOMPARE(lmChange, lms.at(2));

    QTest::qWait(10);
    QCOMPARE(spyAdd.count(), 1);
    QCOMPARE(spyChange.count(), 1);
    QCOMPARE(spyRemove.count(), 0);
    QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkId> >().at(0), firstLandmarkId);
    QCOMPARE(spyChange.at(0).at(0).value<QList<QLandmarkId> >().at(0), thirdLandmarkId);
    spyAdd.clear();
    spyChange.clear();

    //try saving multiple new landmarks
    QLandmark lmNew1;
    lmNew1.setName("LMNew1");
    lmNew1.setDescription("LMNew1 description");

    QLandmark lmNew2;
    lmNew2.setName("LMNew2");
    coordinate.setLatitude(80);
    coordinate.setLongitude(80);
    lmNew2.setCoordinate(coordinate);

    QLandmark lmNew3;
    lmNew3.setName("LMNew3");
    address.clear();
    address.setCountry("LMNew3 Country");
    lmNew3.setAddress(address);

    lms.clear();
    lms << lmNew1 << lmNew2 << lmNew3;
    QVERIFY(doSave(type, &lms,QLandmarkManager::NoError, &errorMap));
    QCOMPARE(lms.count(), 3);
    QCOMPARE(errorMap.count(), 0);

    firstLandmarkId = lms.at(0).landmarkId();
    secondLandmarkId = lms.at(1).landmarkId();
    thirdLandmarkId = lms.at(2).landmarkId();

    lmNew1.setLandmarkId(firstLandmarkId);
    lmNew2.setLandmarkId(secondLandmarkId);
    lmNew3.setLandmarkId(thirdLandmarkId);

    QCOMPARE(lmNew1, m_manager->landmark(firstLandmarkId));
    QCOMPARE(lmNew2, m_manager->landmark(secondLandmarkId));
    QCOMPARE(lmNew3, m_manager->landmark(thirdLandmarkId));

    QCOMPARE(lmNew1, lms.at(0));
    QCOMPARE(lmNew2, lms.at(1));
    QCOMPARE(lmNew3, lms.at(2));

    QTest::qWait(10);
    QCOMPARE(spyAdd.count(), 1);
    QCOMPARE(spyChange.count(), 0);
    QCOMPARE(spyRemove.count(), 0);
    QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkId> >().at(0), firstLandmarkId);
    QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkId> >().at(1), secondLandmarkId);
    QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkId> >().at(2), thirdLandmarkId);
    spyAdd.clear();

    //try updating multiple landmarks
    lmNew1.setDescription("");
    lmNew2.setRadius(50);
    coordinate.setLatitude(1);
    coordinate.setLongitude(2);
    lmNew3.setCoordinate(coordinate);
    lms.clear();
    lms << lmNew1 << lmNew2 << lmNew3;
    QVERIFY(doSave(type, &lms, QLandmarkManager::NoError, &errorMap));
    QCOMPARE(lms.count(), 3);
    QCOMPARE(errorMap.count(), 0);

    lmNew1.setLandmarkId(firstLandmarkId);
    lmNew2.setLandmarkId(secondLandmarkId);
    lmNew3.setLandmarkId(thirdLandmarkId);

    QCOMPARE(lmNew1, m_manager->landmark(firstLandmarkId));
    QCOMPARE(lmNew2, m_manager->landmark(secondLandmarkId));
    QCOMPARE(lmNew3, m_manager->landmark(thirdLandmarkId));

    QCOMPARE(lmNew1, lms.at(0));
    QCOMPARE(lmNew2, lms.at(1));
    QCOMPARE(lmNew3, lms.at(2));

    QTest::qWait(10);
    QCOMPARE(spyAdd.count(), 0);
    QCOMPARE(spyChange.count(), 1);
    QCOMPARE(spyRemove.count(), 0);
    QCOMPARE(spyChange.at(0).at(0).value<QList<QLandmarkId> >().at(0), firstLandmarkId);
    QCOMPARE(spyChange.at(0).at(0).value<QList<QLandmarkId> >().at(1), secondLandmarkId);
    QCOMPARE(spyChange.at(0).at(0).value<QList<QLandmarkId> >().at(2), thirdLandmarkId);
    spyChange.clear();

    if (type == "async") {
        //check that errorMap is cleared for async landmark save request
        QLandmarkSaveRequest lmSaveRequest(m_manager);
        QSignalSpy spy(&lmSaveRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
        QSignalSpy spyResults(&lmSaveRequest, SIGNAL(resultsAvailable()));
        lms.clear();
        lms << lmNew << lmBad << lmChange;
        lmSaveRequest.setLandmarks(lms);
        lmSaveRequest.start();
        QVERIFY(waitForAsync(spy, &lmSaveRequest, QLandmarkManager::DoesNotExistError));
        QCOMPARE(lmSaveRequest.errorMap().count(), 1);
        QCOMPARE(lmSaveRequest.errorMap().value(1), QLandmarkManager::DoesNotExistError);
        QCOMPARE(spyResults.count(), 1);
        spyResults.clear();

        lms.clear();
        lms << lmNew1 << lmNew2 << lmNew3;
        lmSaveRequest.setLandmarks(lms);
        lmSaveRequest.start();
        QVERIFY(waitForAsync(spy, &lmSaveRequest, QLandmarkManager::NoError));
        QCOMPARE(lmSaveRequest.errorMap().count(), 0);
        QCOMPARE(spyResults.count(), 1);
    }

    if (type =="sync") {
        //check that the error is cleared on consecutive single saves
        QLandmark lmAlpha;
        QLandmarkId lmAlphaId;
        lmAlphaId.setManagerUri(m_manager->managerUri());
        lmAlphaId.setLocalId("42");
        lmAlpha.setLandmarkId(lmAlphaId);
        QVERIFY(!m_manager->saveLandmark(&lmAlpha));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);

        lmAlphaId.setLocalId("");
        lmAlpha.setLandmarkId(lmAlphaId);
        QVERIFY(m_manager->saveLandmark(&lmAlpha));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);

        //check that the error is cleared on consecutive multiple saves

        QLandmark lmBeta;
        QLandmarkId lmBetaId;
        lmBetaId.setManagerUri(m_manager->managerUri());
        lmBetaId.setLocalId("42");
        lmBeta.setLandmarkId(lmBetaId);

        QLandmark lmGamma;
        lms.clear();
        lms << lmBeta << lmGamma;

        QVERIFY(!m_manager->saveLandmarks(&lms));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        lmBetaId.setLocalId("");
        lmBeta.setLandmarkId(lmBetaId);
        lms.clear();
        lms << lmBeta << lmGamma;

        QVERIFY(m_manager->saveLandmarks(&lms));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    }
}

void tst_QLandmarkManager::saveLandmark_data() {
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}


void tst_QLandmarkManager::removeCategory() {
    QFETCH(QString, type);
    QSignalSpy spyLmAdd(m_manager, SIGNAL(landmarksAdded(QList<QLandmarkId>)));
    QSignalSpy spyLmChange(m_manager, SIGNAL(landmarksChanged(QList<QLandmarkId>)));
    QSignalSpy spyLmRemove(m_manager, SIGNAL(landmarksRemoved(QList<QLandmarkId>)));
    QSignalSpy spyCatAdd(m_manager, SIGNAL(categoriesAdded(QList<QLandmarkCategoryId>)));
    QSignalSpy spyCatChange(m_manager, SIGNAL(categoriesChanged(QList<QLandmarkCategoryId>)));
    QSignalSpy spyCatRemove(m_manager, SIGNAL(categoriesRemoved(QList<QLandmarkCategoryId>)));

    QLandmarkCategoryId id1;
    QVERIFY(doSingleCategoryRemove(type,id1,QLandmarkManager::DoesNotExistError));

    id1.setManagerUri(m_manager->managerUri());
    id1.setLocalId("100");
    QVERIFY(doSingleCategoryRemove(type,id1,QLandmarkManager::DoesNotExistError));

    id1.setManagerUri("different.manager");
    id1.setLocalId("100");
    QVERIFY(doSingleCategoryRemove(type,id1,QLandmarkManager::DoesNotExistError));
    QTest::qWait(10);
    QCOMPARE(spyLmAdd.count(), 0);
    QCOMPARE(spyLmChange.count(), 0);
    QCOMPARE(spyLmRemove.count(), 0);
    QCOMPARE(spyCatAdd.count(), 0);
    QCOMPARE(spyCatChange.count(), 0);
    QCOMPARE(spyCatRemove.count(), 0);

    QLandmarkCategory cat1;
    cat1.setName("CAT1");
    QVERIFY(m_manager->saveCategory(&cat1));

    QVERIFY(doSingleCategoryRemove(type, cat1.categoryId(),QLandmarkManager::NoError));
    QCOMPARE(m_manager->category(cat1.categoryId()), QLandmarkCategory());
    QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);

    QTest::qWait(10);
    QCOMPARE(spyLmAdd.count(), 0);
    QCOMPARE(spyLmChange.count(), 0);
    QCOMPARE(spyLmRemove.count(), 0);
    QCOMPARE(spyCatAdd.count(), 1);
    QCOMPARE(spyCatChange.count(), 0);
    QCOMPARE(spyCatRemove.count(), 1);
    QCOMPARE(spyCatRemove.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(0), cat1.categoryId());
    spyCatAdd.clear();
    spyCatRemove.clear();

    // effect on landmarks
    cat1.setCategoryId(QLandmarkCategoryId());
    QVERIFY(m_manager->saveCategory(&cat1));

    QLandmarkCategory cat2;
    cat2.setName("CAT2");
    QVERIFY(m_manager->saveCategory(&cat2));

    QLandmarkCategory cat3;
    cat3.setName("CAT3");
    QVERIFY(m_manager->saveCategory(&cat3));

    QLandmark lm1;
    lm1.setName("LM1");
    QVERIFY(m_manager->saveLandmark(&lm1));
    QCOMPARE(lm1, m_manager->landmark(lm1.landmarkId()));

    lm1.addCategoryId(cat3.categoryId());
    lm1.addCategoryId(cat2.categoryId());
    lm1.addCategoryId(cat1.categoryId());

    QVERIFY(m_manager->saveLandmark(&lm1));
    QCOMPARE(lm1, m_manager->landmark(lm1.landmarkId()));
    QCOMPARE(lm1.categoryIds().count(), 3);
    QVERIFY(lm1.categoryIds().contains(cat1.categoryId()));
    QVERIFY(lm1.categoryIds().contains(cat2.categoryId()));
    QVERIFY(lm1.categoryIds().contains(cat3.categoryId()));

    QLandmark lm2;
    lm2.setName("LM2");
    lm2.addCategoryId(cat2.categoryId());
    QVERIFY(m_manager->saveLandmark(&lm2));

    QCOMPARE(lm2, m_manager->landmark(lm2.landmarkId()));
    QCOMPARE(lm2.categoryIds().count(), 1);
    QVERIFY(lm2.categoryIds().contains(cat2.categoryId()));

    QVERIFY(doSingleCategoryRemove(type, cat2.categoryId(),QLandmarkManager::NoError));

    QTest::qWait(10);
#ifdef Q_OS_SYMBIAN
    QCOMPARE(spyLmAdd.count(), 2);
#else
    QCOMPARE(spyLmAdd.count(), 1);
#endif

    if (type == "sync") {
        QCOMPARE(spyLmChange.count(), 1);
        QCOMPARE(spyLmChange.at(0).at(0).value<QList<QLandmarkId> >().count(), 3);
        QCOMPARE(spyLmChange.at(0).at(0).value<QList<QLandmarkId> >().at(0), lm1.landmarkId());
        QCOMPARE(spyLmChange.at(0).at(0).value<QList<QLandmarkId> >().at(1), lm1.landmarkId());
        QCOMPARE(spyLmChange.at(0).at(0).value<QList<QLandmarkId> >().at(2), lm2.landmarkId());
    }
    else if (type == "async") {
        QCOMPARE(spyLmChange.count(), 2);
        QCOMPARE(spyLmChange.at(0).at(0).value<QList<QLandmarkId> >().at(0), lm1.landmarkId());
        QCOMPARE(spyLmChange.at(1).at(0).value<QList<QLandmarkId> >().at(0), lm1.landmarkId());
        QCOMPARE(spyLmChange.at(1).at(0).value<QList<QLandmarkId> >().at(1), lm2.landmarkId());
    }

    QCOMPARE(spyLmRemove.count(), 0);
#ifdef Q_OS_SYMBIAN
    QCOMPARE(spyCatAdd.count(), 3);
#else
    QCOMPARE(spyCatAdd.count(), 1);
#endif
    QCOMPARE(spyCatChange.count(), 0);
    QCOMPARE(spyCatRemove.count(), 1);
    QCOMPARE(spyCatRemove.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(0), cat2.categoryId());
    spyLmAdd.clear();
    spyLmChange.clear();
    spyCatAdd.clear();
    spyCatRemove.clear();

    QLandmark lm1New = m_manager->landmark(lm1.landmarkId());
    QVERIFY(lm1New != lm1);
    QCOMPARE(lm1.categoryIds().size(), 3);
    QCOMPARE(lm1New.categoryIds().size(), 2);
    QVERIFY(lm1New.categoryIds().contains(cat1.categoryId()));
    QVERIFY(lm1New.categoryIds().contains(cat3.categoryId()));

    QLandmark lm2New = m_manager->landmark(lm2.landmarkId());
    QVERIFY(lm2New != lm2);
    QCOMPARE(lm2.categoryIds().size(), 1);
    QCOMPARE(lm2New.categoryIds().size(), 0);


    QLandmarkCategory cat4;
    cat4.setName("CAT4");
    cat4.setIconUrl(QUrl("CAT4 iconUrl"));
    QVERIFY(m_manager->saveCategory(&cat4));

    QLandmarkCategory cat5;
    cat5.setName("CAT5");
    cat5.setIconUrl(QUrl("CAT5 iconUrl"));
    // Disable custom attributes cat5.setCustomAttribute("five", 5);
    QVERIFY(m_manager->saveCategory(&cat5));

    QLandmarkCategory cat6;
    cat6.setName("LM6");
    cat6.setIconUrl(QUrl("CAT6 iconUrl"));
    //Disable custom attributes cat6.setCustomAttribute("six", 6);
    QVERIFY(m_manager->saveCategory(&cat6));

#ifndef Q_OS_SYMBIAN
    {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","testing");
    db.setDatabaseName("test.db");
    db.open();
    QSqlQuery query(db);

    query.exec(QString("SELECT * FROM category_attribute WHERE categoryId=%1").arg(cat4.categoryId().localId()));
    QVERIFY(query.next());
    query.exec(QString("SELECT * FROM category_attribute WHERE categoryId=%1").arg(cat5.categoryId().localId()));
    QVERIFY(query.next());
    query.exec(QString("SELECT * FROM category_attribute WHERE categoryId=%1").arg(cat6.categoryId().localId()));
    QVERIFY(query.next());

    query.finish();
#endif

    QVERIFY(m_manager->removeCategory(cat5));
    QCOMPARE(m_manager->category(cat4.categoryId()),cat4);
    QCOMPARE(m_manager->category(cat5.categoryId()), QLandmarkCategory());
    QCOMPARE(m_manager->category(cat6.categoryId()),cat6);

#ifndef Q_OS_SYMBIAN
    query.exec(QString("SELECT * FROM category_attribute WHERE categoryId=%1").arg(cat4.categoryId().localId()));
    QVERIFY(query.next());
    query.exec(QString("SELECT * FROM category_attribute WHERE categoryId=%1").arg(cat5.categoryId().localId()));
    QVERIFY(!query.next());
    query.exec(QString("SELECT * FROM category_attribute WHERE categoryId=%1").arg(cat6.categoryId().localId()));
    QVERIFY(query.next());
    }
    QSqlDatabase::removeDatabase("testing");
#endif

    QTest::qWait(10);
    QCOMPARE(spyLmAdd.count(), 0);
    QCOMPARE(spyLmChange.count(), 0);
    QCOMPARE(spyLmRemove.count(), 0);
#ifdef Q_OS_SYMBIAN
    QCOMPARE(spyCatAdd.count(), 3);
#else
    QCOMPARE(spyCatAdd.count(), 1);
#endif
    QCOMPARE(spyCatChange.count(), 0);
    QCOMPARE(spyCatRemove.count(), 1);

    QCOMPARE(spyCatRemove.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(0), cat5.categoryId());
    spyCatAdd.clear();
    spyCatRemove.clear();

    if (type == "async") {
    //try removing multiple categories
    QLandmarkCategory catA;
    catA.setName("CAT-A");
    m_manager->saveCategory(&catA);

    QLandmarkCategory catB;
    catB.setName("CAT-B");
    m_manager->saveCategory(&catB);

    QLandmarkCategory catC;
    catC.setName("CAT-C");
    m_manager->saveCategory(&catC);

    QLandmarkCategoryId catIdNotExist;
    catIdNotExist.setManagerUri(m_manager->managerUri());
    catIdNotExist.setLocalId("5000");

    QLandmarkCategoryId catIdNotExist2;

    QList<QLandmarkCategoryId> catIds;
    catIds << catA.categoryId() << catIdNotExist << catC.categoryId() << catIdNotExist2;

    QLandmarkCategoryRemoveRequest removeRequest(m_manager);
    QSignalSpy spy(&removeRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
    QSignalSpy spyResult(&removeRequest, SIGNAL(resultsAvailable()));
    removeRequest.setCategoryIds(catIds);
    removeRequest.start();
    QVERIFY(waitForAsync(spy, &removeRequest,QLandmarkManager::DoesNotExistError,1000));
    QCOMPARE(removeRequest.errorMap().count(),2);
    QCOMPARE(removeRequest.errorMap().keys().at(0),1);
    QCOMPARE(removeRequest.errorMap().keys().at(1),3);
    QCOMPARE(removeRequest.errorMap().value(1), QLandmarkManager::DoesNotExistError);
    QCOMPARE(removeRequest.errorMap().value(3), QLandmarkManager::DoesNotExistError);

    QCOMPARE(m_manager->category(catA.categoryId()), QLandmarkCategory());
    QCOMPARE(m_manager->category(catB.categoryId()), catB);
    QCOMPARE(m_manager->category(catC.categoryId()), QLandmarkCategory());

    QCOMPARE(spyResult.count(), 1);
    spyResult.clear();

    QTest::qWait(10);

    QCOMPARE(spyLmAdd.count(), 0);
    QCOMPARE(spyLmChange.count(), 0);
    QCOMPARE(spyLmRemove.count(), 0);
#ifdef Q_OS_SYMBIAN
    QCOMPARE(spyCatAdd.count(), 3);
#else
    QCOMPARE(spyCatAdd.count(), 1);
#endif
    QCOMPARE(spyCatChange.count(), 0);
    QCOMPARE(spyCatRemove.count(), 1);
    QCOMPARE(spyCatRemove.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(0), catA.categoryId());
    QCOMPARE(spyCatRemove.at(0).at(0).value<QList<QLandmarkCategoryId> >().at(1), catC.categoryId());
    spyCatAdd.clear();
    spyCatRemove.clear();

    //check that error map will be cleared
    catA.setCategoryId(QLandmarkCategoryId());
    catC.setCategoryId(QLandmarkCategoryId());
    QVERIFY(m_manager->saveCategory(&catA));
    QVERIFY(m_manager->saveCategory(&catC));

    catIds.clear();
    catIds << catC.categoryId() << catB.categoryId() << catA.categoryId();
    removeRequest.setCategoryIds(catIds);
    removeRequest.start();
    QVERIFY(waitForAsync(spy, &removeRequest,QLandmarkManager::NoError,1000));
    QCOMPARE(removeRequest.errorMap().count(),0);
    QCOMPARE(m_manager->category(catA.categoryId()), QLandmarkCategory());
    QCOMPARE(m_manager->category(catB.categoryId()), QLandmarkCategory());
    QCOMPARE(m_manager->category(catC.categoryId()), QLandmarkCategory());

    QTest::qWait(10);

    QCOMPARE(spyLmAdd.count(), 0);
    QCOMPARE(spyLmChange.count(), 0);
    QCOMPARE(spyLmRemove.count(), 0);
#ifdef Q_OS_SYMBIAN
    QCOMPARE(spyCatAdd.count(), 2);
#else
    QCOMPARE(spyCatAdd.count(), 1);
#endif
    QCOMPARE(spyCatChange.count(), 0);
    QCOMPARE(spyCatRemove.count(), 1);
    QCOMPARE(spyCatRemove.at(0).at(0).value<QList<QLandmarkCategoryId> >().count(),3);
    QVERIFY(spyCatRemove.at(0).at(0).value<QList<QLandmarkCategoryId> >().contains(catC.categoryId()));
    QVERIFY(spyCatRemove.at(0).at(0).value<QList<QLandmarkCategoryId> >().contains(catB.categoryId()));
    QVERIFY(spyCatRemove.at(0).at(0).value<QList<QLandmarkCategoryId> >().contains(catA.categoryId()));
    spyCatAdd.clear();
    spyCatRemove.clear();
    }

    if (type == "sync") {
        //ensure that error is cleared, for consecutive sync removes for category objects
        QLandmarkCategory catAlpha;
        catAlpha.setName("CAT-Alpha");
        QVERIFY(m_manager->saveCategory(&catAlpha));

        QLandmarkCategory catBeta;//beta doesn't exist

        QVERIFY(!m_manager->removeCategory(catBeta));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QVERIFY(m_manager->removeCategory(catAlpha));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);

        //try consecutive removals for category ids
        catAlpha.setCategoryId(QLandmarkCategoryId());
        QVERIFY(m_manager->saveCategory(&catAlpha));

        QVERIFY(!m_manager->removeCategory(QLandmarkCategoryId()));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QVERIFY(m_manager->removeCategory(catAlpha.categoryId()));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    }
}

void tst_QLandmarkManager::removeCategory_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::removeLandmark()
{
    QFETCH(QString, type);
    QSignalSpy spyLmAdd(m_manager, SIGNAL(landmarksAdded(QList<QLandmarkId>)));
    QSignalSpy spyLmChange(m_manager, SIGNAL(landmarksChanged(QList<QLandmarkId>)));
    QSignalSpy spyLmRemove(m_manager, SIGNAL(landmarksRemoved(QList<QLandmarkId>)));
    QSignalSpy spyCatAdd(m_manager, SIGNAL(categoriesAdded(QList<QLandmarkCategoryId>)));
    QSignalSpy spyCatChange(m_manager, SIGNAL(categoriesChanged(QList<QLandmarkCategoryId>)));
    QSignalSpy spyCatRemove(m_manager, SIGNAL(categoriesRemoved(QList<QLandmarkCategoryId>)));

    QLandmarkId id1;
    QVERIFY(doSingleLandmarkRemove(type,id1,QLandmarkManager::DoesNotExistError));

    id1.setManagerUri(m_manager->managerUri());
    id1.setLocalId("100");
    QVERIFY(doSingleLandmarkRemove(type,id1,QLandmarkManager::DoesNotExistError));

    id1.setManagerUri("different.manager");
    id1.setLocalId("100");
    QVERIFY(doSingleLandmarkRemove(type,id1,QLandmarkManager::DoesNotExistError));
    QTest::qWait(10);
    QCOMPARE(spyLmAdd.count(), 0);
    QCOMPARE(spyLmChange.count(), 0);
    QCOMPARE(spyLmRemove.count(), 0);
    QCOMPARE(spyCatAdd.count(), 0);
    QCOMPARE(spyCatChange.count(), 0);
    QCOMPARE(spyCatRemove.count(), 0);

    QLandmark lm1;
    lm1.setName("LM1");
    QVERIFY(m_manager->saveLandmark(&lm1));
    QVERIFY(doSingleLandmarkRemove(type, lm1.landmarkId(), QLandmarkManager::NoError));
    QCOMPARE(m_manager->landmark(lm1.landmarkId()), QLandmark());
    QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);

    QTest::qWait(10);
    QCOMPARE(spyLmAdd.count(), 1);
    QCOMPARE(spyLmChange.count(), 0);
    QCOMPARE(spyLmRemove.count(), 1);
    QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(0), lm1.landmarkId());
    QCOMPARE(spyCatAdd.count(), 0);
    QCOMPARE(spyCatChange.count(), 0);
    QCOMPARE(spyCatRemove.count(), 0);
    spyLmAdd.clear();
    spyLmRemove.clear();

    //try removing multiple landmarks
    lm1.setLandmarkId(QLandmarkId());
    QVERIFY(m_manager->saveLandmark(&lm1));

    QLandmark lm2;
    lm2.setName("LM2");
    QVERIFY(m_manager->saveLandmark(&lm2));

    QLandmark lm3;
    lm3.setName("LM3");
    QVERIFY(m_manager->saveLandmark(&lm3));

    QLandmark lm4;
    lm4.setName("LM4");
    QVERIFY(m_manager->saveLandmark(&lm4));

    QTest::qWait(10);
    QCOMPARE(spyLmAdd.count(), 1);
    QCOMPARE(spyLmChange.count(), 0);
    QCOMPARE(spyLmRemove.count(), 0);
    QCOMPARE(spyCatAdd.count(), 0);
    QCOMPARE(spyCatChange.count(), 0);
    QCOMPARE(spyCatRemove.count(), 0);
    spyLmAdd.clear();

    QLandmarkId lmIdNotExist;
    lmIdNotExist.setManagerUri(m_manager->managerUri());
    lmIdNotExist.setLocalId("5000");

    QLandmarkId lmIdNotExist2;

    QList<QLandmarkId> lmIds;
    lmIds << lm1.landmarkId() << lmIdNotExist << lm3.landmarkId() << lmIdNotExist2 << lm4.landmarkId();
    QMap<int, QLandmarkManager::Error> errorMap;
    QLandmarkRemoveRequest removeRequest(m_manager);
    QSignalSpy spy(&removeRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
    QSignalSpy spyResult(&removeRequest, SIGNAL(resultsAvailable()));

    if (type=="sync") {
        //remove landmarks without an errormap
        QVERIFY(!m_manager->removeLandmarks(lmIds));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(m_manager->landmark(lm1.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lm2.landmarkId()), lm2);
        QCOMPARE(m_manager->landmark(lm3.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lm4.landmarkId()), QLandmark());

        QTest::qWait(10);
        QCOMPARE(spyLmAdd.count(), 0);
        QCOMPARE(spyLmChange.count(), 0);
        QCOMPARE(spyLmRemove.count(), 1);
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().count(), 3);
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(0), lm1.landmarkId());
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(1), lm3.landmarkId());
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(2), lm4.landmarkId());
        QCOMPARE(spyCatAdd.count(), 0);
        QCOMPARE(spyCatChange.count(), 0);

        spyLmAdd.clear();
        spyLmRemove.clear();

        //try removing landmarks with an error map
        lm1.setLandmarkId(QLandmarkId());
        QVERIFY(m_manager->saveLandmark(&lm1));
        lm3.setLandmarkId(QLandmarkId());
        QVERIFY(m_manager->saveLandmark(&lm3));
        lm4.setLandmarkId(QLandmarkId());
        QVERIFY(m_manager->saveLandmark(&lm4));

        lmIds.clear();
        lmIds << lm1.landmarkId() << lmIdNotExist << lm3.landmarkId() << lmIdNotExist2 << lm4.landmarkId();

        QVERIFY(!m_manager->removeLandmarks(lmIds, &errorMap));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(m_manager->landmark(lm1.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lm2.landmarkId()), lm2);
        QCOMPARE(m_manager->landmark(lm3.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lm4.landmarkId()), QLandmark());

        QCOMPARE(errorMap.count(), 2);
        QCOMPARE(errorMap.keys().at(0), 1);
        QCOMPARE(errorMap.keys().at(1), 3);
        QCOMPARE(errorMap.value(1), QLandmarkManager::DoesNotExistError);
        QCOMPARE(errorMap.value(3), QLandmarkManager::DoesNotExistError);

        QTest::qWait(10);
        QCOMPARE(spyLmAdd.count(), 1);
        QCOMPARE(spyLmAdd.at(0).at(0).value<QList<QLandmarkId> >().count(), 3);
        QCOMPARE(spyLmAdd.at(0).at(0).value<QList<QLandmarkId> >().at(0), lm1.landmarkId());
        QCOMPARE(spyLmAdd.at(0).at(0).value<QList<QLandmarkId> >().at(1), lm3.landmarkId());
        QCOMPARE(spyLmAdd.at(0).at(0).value<QList<QLandmarkId> >().at(2), lm4.landmarkId());
        QCOMPARE(spyLmChange.count(), 0);
        QCOMPARE(spyLmRemove.count(), 1);
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().count(), 3);

        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(0), lm1.landmarkId());
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(1), lm3.landmarkId());
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(2), lm4.landmarkId());
        QCOMPARE(spyCatAdd.count(), 0);
        QCOMPARE(spyCatChange.count(), 0);
        QCOMPARE(spyCatRemove.count(), 0);
        spyLmAdd.clear();
        spyLmRemove.clear();
    } else if (type == "async") {
        removeRequest.setLandmarkIds(lmIds);
        removeRequest.start();
        QVERIFY(waitForAsync(spy, &removeRequest,QLandmarkManager::DoesNotExistError,1000));
        QCOMPARE(spyResult.count(), 1);
        spyResult.clear();

        QCOMPARE(removeRequest.errorMap().count(),2);
        QCOMPARE(removeRequest.errorMap().keys().at(0),1);
        QCOMPARE(removeRequest.errorMap().keys().at(1),3);
        QCOMPARE(removeRequest.errorMap().value(1), QLandmarkManager::DoesNotExistError);
        QCOMPARE(removeRequest.errorMap().value(3), QLandmarkManager::DoesNotExistError);

        QCOMPARE(m_manager->landmark(lm1.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lm2.landmarkId()), lm2);
        QCOMPARE(m_manager->landmark(lm3.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lm4.landmarkId()), QLandmark());

        QTest::qWait(10);
        QCOMPARE(spyLmAdd.count(), 0);
        QCOMPARE(spyLmChange.count(), 0);
        QCOMPARE(spyLmRemove.count(), 1);
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().count(), 3);
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(0), lm1.landmarkId());
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(1), lm3.landmarkId());
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(2), lm4.landmarkId());
        QCOMPARE(spyCatAdd.count(), 0);
        QCOMPARE(spyCatChange.count(), 0);
        spyLmRemove.clear();
    }

    QLandmark lmA;
    lmA.setName("LM-A");
    QVERIFY(m_manager->saveLandmark(&lmA));

    QLandmark lmB;
    lmB.setName("LM-B");
    QVERIFY(m_manager->saveLandmark(&lmB));

    QLandmark lmC;
    lmB.setName("LM-C");
    QVERIFY(m_manager->saveLandmark(&lmC));
    QTest::qWait(10);
    spyLmAdd.clear();

    lmIds.clear();
    lmIds << lmA.landmarkId() << lmB.landmarkId() << lmC.landmarkId();

    //ensure that the errorMap is cleared
    if (type == "sync") {
        QVERIFY(m_manager->removeLandmarks(lmIds, &errorMap));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
        QCOMPARE(errorMap.count(), 0);
        QCOMPARE(m_manager->landmark(lmA.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmB.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmC.landmarkId()), QLandmark());

        QTest::qWait(10);
        QCOMPARE(spyLmAdd.count(), 0);
        QCOMPARE(spyLmChange.count(), 0);
        QCOMPARE(spyLmRemove.count(), 1);
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().count(), 3);
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(0), lmA.landmarkId());
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(1), lmB.landmarkId());
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(2), lmC.landmarkId());
        QCOMPARE(spyCatAdd.count(), 0);
        QCOMPARE(spyCatChange.count(), 0);
        QCOMPARE(spyCatRemove.count(), 0);
        spyLmRemove.clear();
    } else if (type == "async") {
        removeRequest.setLandmarkIds(lmIds);
        removeRequest.start();
        QVERIFY(waitForAsync(spy, &removeRequest,QLandmarkManager::NoError,1000));

        QCOMPARE(spyResult.count(), 1);
        spyResult.clear();

        QCOMPARE(removeRequest.errorMap().count(), 0);
        QCOMPARE(m_manager->landmark(lmA.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmB.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmC.landmarkId()), QLandmark());

        QTest::qWait(10);
        QCOMPARE(spyLmAdd.count(), 0);
        QCOMPARE(spyLmChange.count(), 0);
        QCOMPARE(spyLmRemove.count(), 1);
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().count(), 3);
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(0), lmA.landmarkId());
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(1), lmB.landmarkId());
        QCOMPARE(spyLmRemove.at(0).at(0).value<QList<QLandmarkId> >().at(2), lmC.landmarkId());
        QCOMPARE(spyCatAdd.count(), 0);
        QCOMPARE(spyCatChange.count(), 0);
        QCOMPARE(spyCatRemove.count(), 0);
        spyLmRemove.clear();
    } else {
        qFatal("Unknown test row type");
    }

    if (type == "sync") {
        //check that consecutive removes by landmark object clears error and error map.
         lmA.setLandmarkId(QLandmarkId());
         lmB.setLandmarkId(QLandmarkId());
         lmC.setLandmarkId(QLandmarkId());

        QLandmark lmD;
        lmD.setName("lmD");
        QLandmark lmE;
        lmE.setName("lmE");
        QLandmark lmF;
        lmF.setName("lmF");
        QLandmark lmG;
        lmG.setName("lmG");
        QLandmark lmH;
        lmH.setName("lmH");

        QVERIFY(m_manager->saveLandmark(&lmA));
        QVERIFY(m_manager->saveLandmark(&lmB));
        QVERIFY(m_manager->saveLandmark(&lmD));
        QVERIFY(m_manager->saveLandmark(&lmE));
        QVERIFY(m_manager->saveLandmark(&lmF));
        QVERIFY(m_manager->saveLandmark(&lmG));
        QVERIFY(m_manager->saveLandmark(&lmH));

        QTest::qWait(10);
        spyLmAdd.clear();

        QVERIFY(m_manager->removeLandmark(lmA));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);

        QVERIFY(m_manager->removeLandmark(lmB));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);

        //remove multiple landmarks
        QList<QLandmark> lms;
        lms << lmC << lmD;

        //try with an error map
        QMap<int, QLandmarkManager::Error> errorMap;
        QVERIFY(!m_manager->removeLandmarks(lms, &errorMap));

        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(errorMap.keys().count(),1);

        QCOMPARE(errorMap.value(0), QLandmarkManager::DoesNotExistError);

        //ensure the error map is cleared
        lms.clear();
        lms << lmE << lmF;
        QVERIFY(m_manager->removeLandmarks(lms, &errorMap));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
        QCOMPARE(errorMap.keys().count(),0);

        //try without an error map
        lms.clear();
        lms << lmG << lmH;

        QVERIFY(m_manager->removeLandmarks(lms));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);

        QCOMPARE(m_manager->landmark(lmA.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmB.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmD.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmE.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmE.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmF.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmF.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmG.landmarkId()), QLandmark());

        //check that consecutive landmark removes by id clears error and error map.
        lmA.setLandmarkId(QLandmarkId());
        lmB.setLandmarkId(QLandmarkId());
        lmD.setLandmarkId(QLandmarkId());
        lmE.setLandmarkId(QLandmarkId());
        lmF.setLandmarkId(QLandmarkId());
        lmG.setLandmarkId(QLandmarkId());
        lmH.setLandmarkId(QLandmarkId());

        QVERIFY(m_manager->saveLandmark(&lmA));
        QVERIFY(m_manager->saveLandmark(&lmB));
        QVERIFY(m_manager->saveLandmark(&lmD));
        QVERIFY(m_manager->saveLandmark(&lmE));
        QVERIFY(m_manager->saveLandmark(&lmF));
        QVERIFY(m_manager->saveLandmark(&lmG));
        QVERIFY(m_manager->saveLandmark(&lmH));

        QTest::qWait(10);
        spyLmAdd.clear();

        QVERIFY(m_manager->removeLandmark(lmA.landmarkId()));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
        QCOMPARE(m_manager->landmark(lmA.landmarkId()), QLandmark());
        QVERIFY(m_manager->removeLandmark(lmB));

        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
        QCOMPARE(m_manager->landmark(lmB.landmarkId()), QLandmark());

        //remove multiple landmarks
        QList<QLandmarkId> lmIds;
        lmIds.clear();
        lmIds << lmC.landmarkId() << lmD.landmarkId();

        //try with an error map
        QVERIFY(!m_manager->removeLandmarks(lmIds, &errorMap));

        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);
        QCOMPARE(errorMap.keys().count(),1);

        QCOMPARE(errorMap.value(0), QLandmarkManager::DoesNotExistError);
        QCOMPARE(m_manager->landmark(lmD.landmarkId()), QLandmark());

        //ensure the error map is cleared
        lmIds.clear();
        lmIds << lmE.landmarkId() << lmF.landmarkId();
        QVERIFY(m_manager->removeLandmarks(lmIds, &errorMap));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
        QCOMPARE(errorMap.keys().count(),0);
        QCOMPARE(m_manager->landmark(lmE.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmF.landmarkId()), QLandmark());

        //try without an error map
        lmIds.clear();
        lmIds<< lmG.landmarkId() << lmH.landmarkId();

        QVERIFY(m_manager->removeLandmarks(lmIds));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
        QCOMPARE(m_manager->landmark(lmF.landmarkId()), QLandmark());
        QCOMPARE(m_manager->landmark(lmG.landmarkId()), QLandmark());
    }
}

void tst_QLandmarkManager::removeLandmark_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::categories()
{
    QFETCH(QString, type);
    QLandmarkCategory catA;
    catA.setName("CAT-A");
    catA.setIconUrl(QUrl("CAT-A url"));

    QLandmarkCategory catB;
    catB.setName("CAT-B");
    catB.setIconUrl(QUrl("CAT-B url"));

    QLandmarkCategory catC;
    catC.setName("CAT-C");
    catC.setIconUrl(QUrl("CAT-C url"));

    QLandmarkCategory catD;
    catD.setName("CAT-D");
    catD.setIconUrl(QUrl("CAT-D url"));

    QLandmarkCategory catE;
    catE.setName("CAT-E");
    catE.setIconUrl(QUrl("CAT-E url"));

    QVERIFY(m_manager->saveCategory(&catE));
    QVERIFY(m_manager->saveCategory(&catD));
    QVERIFY(m_manager->saveCategory(&catA));
    QVERIFY(m_manager->saveCategory(&catC));
    QVERIFY(m_manager->saveCategory(&catB));

    QList<QLandmarkCategory> cats;

    //check retrieving categories with default params
    if (type == "sync") {
        cats = m_manager->categories();
    }
    else if (type == "async") {
        QLandmarkCategoryFetchRequest fetchRequest(m_manager);
        QSignalSpy spy(&fetchRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
        fetchRequest.start();
        QVERIFY(waitForAsync(spy, &fetchRequest,QLandmarkManager::NoError,100));
        cats = fetchRequest.categories();
    }

    QCOMPARE(cats.count(), 5);
    QCOMPARE(cats.at(0), catA);
    QCOMPARE(cats.at(1), catB);
    QCOMPARE(cats.at(2), catC);
    QCOMPARE(cats.at(3), catD);
    QCOMPARE(cats.at(4), catE);

    QLandmarkNameSort nameSort;
    nameSort.setCaseSensitivity(Qt::CaseInsensitive);
    nameSort.setDirection(Qt::AscendingOrder);
    QVERIFY(doCategoryFetch(type, -1, 0, nameSort, &cats, QLandmarkManager::NoError));
    QCOMPARE(cats.count(), 5);
    QCOMPARE(cats.at(0), catA);
    QCOMPARE(cats.at(1), catB);
    QCOMPARE(cats.at(2), catC);
    QCOMPARE(cats.at(3), catD);
    QCOMPARE(cats.at(4), catE);

    //try descending order
    nameSort.setDirection(Qt::DescendingOrder);
    QVERIFY(doCategoryFetch(type, -1, 0, nameSort, &cats, QLandmarkManager::NoError));
    QCOMPARE(cats.count(), 5);
    QCOMPARE(cats.at(0), catE);
    QCOMPARE(cats.at(1), catD);
    QCOMPARE(cats.at(2), catC);
    QCOMPARE(cats.at(3), catB);
    QCOMPARE(cats.at(4), catA);

    //try with a limit of 0
    nameSort.setDirection(Qt::AscendingOrder);
    QVERIFY(doCategoryFetch(type, 0, 0, nameSort, &cats, QLandmarkManager::NoError));
    QCOMPARE(cats.count(),0);

    //try a limit as large as the number of categories
    QVERIFY(doCategoryFetch(type, 5, 0, nameSort, &cats, QLandmarkManager::NoError));
    QCOMPARE(cats.count(),5);
    QCOMPARE(cats.at(0), catA);
    QCOMPARE(cats.at(1), catB);
    QCOMPARE(cats.at(2), catC);
    QCOMPARE(cats.at(3), catD);
    QCOMPARE(cats.at(4), catE);

    //try a limit larger than the number of categories
    QVERIFY(doCategoryFetch(type, 7, 0, nameSort, &cats, QLandmarkManager::NoError));
    QCOMPARE(cats.count(),5);
    QCOMPARE(cats.at(0), catA);
    QCOMPARE(cats.at(1), catB);
    QCOMPARE(cats.at(2), catC);
    QCOMPARE(cats.at(3), catD);
    QCOMPARE(cats.at(4), catE);

    //try a negative offset
    QVERIFY(doCategoryFetch(type, -1,-1, nameSort, &cats, QLandmarkManager::NoError));
    QCOMPARE(cats.count(), 5);
    QCOMPARE(cats.at(0), catA);
    QCOMPARE(cats.at(1), catB);
    QCOMPARE(cats.at(2), catC);
    QCOMPARE(cats.at(3), catD);
    QCOMPARE(cats.at(4), catE);

    //try a valid offset
    QVERIFY(doCategoryFetch(type, -1,3, nameSort, &cats, QLandmarkManager::NoError));
    QCOMPARE(cats.count(), 2);
    QCOMPARE(cats.at(0), catD);
    QCOMPARE(cats.at(1), catE);

    //try an offset that's larger than the number of categories
    QVERIFY(doCategoryFetch(type, -1,10, nameSort, &cats, QLandmarkManager::NoError));
    QCOMPARE(cats.count(), 0);

    //try a combination of non default limit and offset values
    QVERIFY(doCategoryFetch(type, 2,2, nameSort, &cats, QLandmarkManager::NoError));
    QCOMPARE(cats.count(), 2);
    QCOMPARE(cats.at(0), catC);
    QCOMPARE(cats.at(1), catD);

    nameSort.setDirection(Qt::DescendingOrder);
    QVERIFY(doCategoryFetch(type, 2,2, nameSort, &cats, QLandmarkManager::NoError));
    QCOMPARE(cats.count(), 2);
    QCOMPARE(cats.at(0), catC);
    QCOMPARE(cats.at(1), catB);

    //check that case sensitivity is not supported.
    nameSort.setCaseSensitivity(Qt::CaseSensitive);
    QVERIFY(doCategoryFetch(type, 2,2, nameSort, &cats, QLandmarkManager::NotSupportedError));
}

void tst_QLandmarkManager::categories_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::filterLandmarksDefault() {
    QFETCH(QString, type);
    QLandmark lm1;
    lm1.setName("LM1");
    QVERIFY(m_manager->saveLandmark(&lm1));

    QLandmark lm2;
    lm2.setName("LM2");
    QVERIFY(m_manager->saveLandmark(&lm2));

    QLandmark lm3;
    lm3.setName("LM3");
    QVERIFY(m_manager->saveLandmark(&lm3));

    QLandmarkFilter filter;
    QList <QLandmark> lms;

    QVERIFY(doFetch(type,filter, &lms,QLandmarkManager::NoError));
    QCOMPARE(lms.at(0), lm1);
    QCOMPARE(lms.at(1), lm2);
    QCOMPARE(lms.at(2), lm3);
}

void tst_QLandmarkManager::filterLandmarksDefault_data() {
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::filterLandmarksName() {
    QFETCH(QString, type);
    QLandmark lm1;
    lm1.setName("Adelaide");
    m_manager->saveLandmark(&lm1);

    QLandmark lm2;
    lm2.setName("Adel");
    m_manager->saveLandmark(&lm2);

    QLandmark lm3;
    lm3.setName("Brisbane");
    m_manager->saveLandmark(&lm3);

    QLandmark lm4;
    lm4.setName("Perth");
    QVERIFY(m_manager->saveLandmark(&lm4));

    QLandmark lm5;
    lm5.setName("Canberra");
    QVERIFY(m_manager->saveLandmark(&lm5));

    QLandmark lm6;
    lm6.setName("Tinberra");
    QVERIFY(m_manager->saveLandmark(&lm6));

    QLandmark lm7;
    lm7.setName("Madelaide");
    QVERIFY(m_manager->saveLandmark(&lm7));

    QLandmark lm8;
    lm8.setName("Terran");
    QVERIFY(m_manager->saveLandmark(&lm8));

    QLandmark lm9;
    lm9.setName("ADEL");
    QVERIFY(m_manager->saveLandmark(&lm9));

    QList<QLandmark> lms;

    //test starts with
    QLandmarkNameFilter nameFilter;
    nameFilter.setName("adel");
    nameFilter.setMatchFlags(QLandmarkFilter::MatchStartsWith);

    QVERIFY(doFetch(type,nameFilter, &lms,QLandmarkManager::NoError));
    QCOMPARE(lms.count(), 3);
    QCOMPARE(lms.at(0), lm1);
    QCOMPARE(lms.at(1), lm2);
    QCOMPARE(lms.at(2), lm9);

    //test contains
    nameFilter.setName("err");
    nameFilter.setMatchFlags(QLandmarkFilter::MatchContains);
    QVERIFY(doFetch(type,nameFilter, &lms,QLandmarkManager::NoError));
    QCOMPARE(lms.count(),3);
    QCOMPARE(lms.at(0), lm5);
    QCOMPARE(lms.at(1), lm6);
    QCOMPARE(lms.at(2), lm8);

       //test fixed string
    nameFilter.setName("adel");
    nameFilter.setMatchFlags(QLandmarkFilter::MatchFixedString);
    QVERIFY(doFetch(type,nameFilter, &lms,QLandmarkManager::NoError));
    QCOMPARE(lms.count(), 2);
    QCOMPARE(lms.at(0), lm2);
    QCOMPARE(lms.at(1), lm9);

    //test match exactly
    nameFilter.setName("Adel");
    nameFilter.setMatchFlags(QLandmarkFilter::MatchExactly);
    QVERIFY(doFetch(type,nameFilter, &lms,QLandmarkManager::NoError));
    QCOMPARE(lms.count(), 1);
    QCOMPARE(lms.at(0), lm2);

    //test no match
    nameFilter.setName("Washington");
    nameFilter.setMatchFlags(QLandmarkFilter::MatchContains);
    QVERIFY(doFetch(type,nameFilter, &lms,QLandmarkManager::NoError));
    QCOMPARE(lms.count(),0);

    //test that can't support case sensitive matching
    nameFilter.setName("ADEL");
    nameFilter.setMatchFlags(QLandmarkFilter::MatchCaseSensitive);
    QVERIFY(doFetch(type,nameFilter, &lms,QLandmarkManager::NotSupportedError));
    QCOMPARE(lms.count(),0);

    nameFilter.setName("ADEL");
    nameFilter.setMatchFlags(QLandmarkFilter::MatchCaseSensitive | QLandmarkFilter::MatchContains);
    QVERIFY(doFetch(type,nameFilter, &lms,QLandmarkManager::NotSupportedError));
    QCOMPARE(lms.count(),0);

    //try landmarks with an emtpy name
    QLandmark lmNoName1;
    QVERIFY(m_manager->saveLandmark(&lmNoName1));

    QLandmark lmNoName2;
    QVERIFY(m_manager->saveLandmark(&lmNoName2));
    nameFilter.setName("");
    nameFilter.setMatchFlags(QLandmarkFilter::MatchFixedString);
    QVERIFY(doFetch(type,nameFilter, &lms, QLandmarkManager::NoError));
    QCOMPARE(lms.count(),2);
    QCOMPARE(lms.at(0), lmNoName1);
    QCOMPARE(lms.at(1), lmNoName2);

    nameFilter.setMatchFlags(QLandmarkFilter::MatchFixedString);
    QVERIFY(doFetch(type,nameFilter, &lms, QLandmarkManager::NoError));
    QCOMPARE(lms.count(),2);
    QCOMPARE(lms.at(0), lmNoName1);
    QCOMPARE(lms.at(1), lmNoName2);

    //try starts with an empty string
    nameFilter.setMatchFlags(QLandmarkFilter::MatchStartsWith);
    QVERIFY(doFetch(type,nameFilter, &lms, QLandmarkManager::NoError));
    QCOMPARE(lms.count(),11);
    QCOMPARE(lms.at(0), lm1);
    QCOMPARE(lms.at(1), lm2);
    QCOMPARE(lms.at(2), lm3);
    QCOMPARE(lms.at(3), lm4);
    QCOMPARE(lms.at(4), lm5);
    QCOMPARE(lms.at(5), lm6);
    QCOMPARE(lms.at(6), lm7);
    QCOMPARE(lms.at(7), lm8);
    QCOMPARE(lms.at(8), lm9);
    QCOMPARE(lms.at(9), lmNoName1);
    QCOMPARE(lms.at(10), lmNoName2);

    //try contains empty string
    nameFilter.setMatchFlags(QLandmarkFilter::MatchContains);
    QVERIFY(doFetch(type,nameFilter, &lms, QLandmarkManager::NoError));
    QCOMPARE(lms.count(),11);
    QCOMPARE(lms.at(0), lm1);
    QCOMPARE(lms.at(1), lm2);
    QCOMPARE(lms.at(2), lm3);
    QCOMPARE(lms.at(3), lm4);
    QCOMPARE(lms.at(4), lm5);
    QCOMPARE(lms.at(5), lm6);
    QCOMPARE(lms.at(6), lm7);
    QCOMPARE(lms.at(7), lm8);
    QCOMPARE(lms.at(8), lm9);
    QCOMPARE(lms.at(9), lmNoName1);
    QCOMPARE(lms.at(10), lmNoName2);

    //try ends with an empty string
    nameFilter.setMatchFlags(QLandmarkFilter::MatchEndsWith);
    QVERIFY(doFetch(type,nameFilter, &lms, QLandmarkManager::NoError));
    QCOMPARE(lms.count(),11);
    QCOMPARE(lms.at(0), lm1);
    QCOMPARE(lms.at(1), lm2);
    QCOMPARE(lms.at(2), lm3);
    QCOMPARE(lms.at(3), lm4);
    QCOMPARE(lms.at(4), lm5);
    QCOMPARE(lms.at(5), lm6);
    QCOMPARE(lms.at(6), lm7);
    QCOMPARE(lms.at(7), lm8);
    QCOMPARE(lms.at(8), lm9);
    QCOMPARE(lms.at(9), lmNoName1);
    QCOMPARE(lms.at(10), lmNoName2);
}

void tst_QLandmarkManager::filterLandmarksName_data() {
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::filterLandmarksProximity() {
    QFETCH(QString, type);
    QList<QGeoCoordinate> greenwhichFilterCoords;
    QList<QGeoCoordinate> datelineFilterCoords;
    QList<QGeoCoordinate> northPoleFilterCoords;
    QList<QGeoCoordinate> southPoleFilterCoords;
    QList<QGeoCoordinate> northFilterCoords;
    QList<QGeoCoordinate> eastFilterCoords;
    QList<QGeoCoordinate> northeastFilterCoords;

    QList<QGeoCoordinate> greenwhichLmCoords;
    QList<QGeoCoordinate> datelineLmCoords;
    QList<QGeoCoordinate> northPoleLmCoords;
    QList<QGeoCoordinate> southPoleLmCoords;
    QList<QGeoCoordinate> northLmCoords;
    QList<QGeoCoordinate> eastLmCoords;
    QList<QGeoCoordinate> northeastLmCoords;

    greenwhichFilterCoords << QGeoCoordinate(-0.1, -0.1);
    greenwhichFilterCoords << QGeoCoordinate(0.1, -0.1);
    greenwhichFilterCoords << QGeoCoordinate(-0.1, 0.1);
    greenwhichFilterCoords << QGeoCoordinate(0.1, 0.1);

    datelineFilterCoords << QGeoCoordinate(-0.1, -179.9);
    datelineFilterCoords << QGeoCoordinate(0.1, -179.9);
    datelineFilterCoords << QGeoCoordinate(-0.1, 179.9);
    datelineFilterCoords << QGeoCoordinate(0.1, 179.9);

    northPoleFilterCoords << QGeoCoordinate(89.9, -179.9);
    northPoleFilterCoords << QGeoCoordinate(89.9, -0.1);
    northPoleFilterCoords << QGeoCoordinate(89.9, 0.1);
    northPoleFilterCoords << QGeoCoordinate(89.9, 179.9);

    southPoleFilterCoords << QGeoCoordinate(-89.9, -179.9);
    southPoleFilterCoords << QGeoCoordinate(-89.9, -0.1);
    southPoleFilterCoords << QGeoCoordinate(-89.9, 0.1);
    southPoleFilterCoords << QGeoCoordinate(-89.9, 179.9);

    eastFilterCoords << QGeoCoordinate(-0.1, 10.0);
    eastFilterCoords << QGeoCoordinate(0.1, 10.0);
    northFilterCoords << QGeoCoordinate(10.0, -0.1);
    northFilterCoords << QGeoCoordinate(10.0, 0.1);
    northeastFilterCoords << QGeoCoordinate(10.0, 10.0);

    greenwhichLmCoords << QGeoCoordinate(-1.0, -1.0);
    greenwhichLmCoords << QGeoCoordinate(1.0, -1.0);
    greenwhichLmCoords << QGeoCoordinate(-1.0, 1.0);
    greenwhichLmCoords << QGeoCoordinate(1.0, 1.0);

    datelineLmCoords << QGeoCoordinate(-1.0, -179.0);
    datelineLmCoords << QGeoCoordinate(1.0, -179.0);
    datelineLmCoords << QGeoCoordinate(-1.0, 179.0);
    datelineLmCoords << QGeoCoordinate(1.0, 179.0);

    northPoleLmCoords << QGeoCoordinate(89.0, -179.0);
    northPoleLmCoords << QGeoCoordinate(89.0, -1.0);
    northPoleLmCoords << QGeoCoordinate(89.0, 1.0);
    northPoleLmCoords << QGeoCoordinate(89.0, 179.0);

    southPoleLmCoords << QGeoCoordinate(-89.0, -179.0);
    southPoleLmCoords << QGeoCoordinate(-89.0, -1.0);
    southPoleLmCoords << QGeoCoordinate(-89.0, 1.0);
    southPoleLmCoords << QGeoCoordinate(-89.0, 179.0);

    eastLmCoords << QGeoCoordinate(-1.0, 11.0);
    eastLmCoords << QGeoCoordinate(1.0, 11.0);
    northLmCoords << QGeoCoordinate(11.0, -1.0);
    northLmCoords << QGeoCoordinate(11.0, 1.0);
    northeastLmCoords << QGeoCoordinate(11.0, 11.0);

    QList<QList<QGeoCoordinate> > coords;
    coords << greenwhichLmCoords;
    coords << datelineLmCoords;

    coords << northPoleLmCoords;
    coords << southPoleLmCoords;
    coords << eastLmCoords;
    coords << northLmCoords;
    coords << northeastLmCoords;

    for (int i = 0; i < coords.size(); ++i) {
        QList<QGeoCoordinate> c = coords.at(i);
        for (int j = 0; j < c.size(); ++j) {
            QLandmark lm;
            lm.setCoordinate(c.at(j));
            QVERIFY(m_manager->saveLandmark(&lm));
        }
    }

    QList<QPair<QList<QGeoCoordinate>, QList<QGeoCoordinate> > > testSets;
    testSets << QPair<QList<QGeoCoordinate>, QList<QGeoCoordinate> >(greenwhichFilterCoords, greenwhichLmCoords);
    testSets << QPair<QList<QGeoCoordinate>, QList<QGeoCoordinate> >(datelineFilterCoords, datelineLmCoords);

    testSets << QPair<QList<QGeoCoordinate>, QList<QGeoCoordinate> >(northPoleFilterCoords, northPoleLmCoords);
    testSets << QPair<QList<QGeoCoordinate>, QList<QGeoCoordinate> >(southPoleFilterCoords, southPoleLmCoords);

    testSets << QPair<QList<QGeoCoordinate>, QList<QGeoCoordinate> >(northFilterCoords, northLmCoords);
    testSets << QPair<QList<QGeoCoordinate>, QList<QGeoCoordinate> >(eastFilterCoords, eastLmCoords);
    testSets << QPair<QList<QGeoCoordinate>, QList<QGeoCoordinate> >(northeastFilterCoords, northeastLmCoords);

    qreal dist = QGeoCoordinate(0.0, 0.0).distanceTo(QGeoCoordinate(5.0, 5.0));

    QList<QLandmark> lms;
    for (int i = 0; i < testSets.size(); ++i) {
        QList<QGeoCoordinate> filterCoords = testSets.at(i).first;
        QList<QGeoCoordinate> lmCoords = testSets.at(i).second;

        for (int j = 0; j < filterCoords.size(); ++j) {
            QLandmarkProximityFilter filter(filterCoords.at(j), dist);


            if (i ==2 || i ==3) { //we're in the testing the north and south poles which is invalid
                QVERIFY(doFetch(type, filter,&lms, QLandmarkManager::BadArgumentError));
                continue;
            } else {
                QVERIFY(doFetch(type, filter,&lms, QLandmarkManager::NoError));
            }

            if (lms.size() != lmCoords.size()) {
                for (int k = 0; k < lms.size(); ++k)
                    qWarning() << "lms" << lms.at(k).coordinate().toString();
                for (int k = 0; k < lmCoords.size(); ++k)
                    qWarning() << "lmCoords" << lmCoords.at(k).toString();
            }

            QCOMPARE(lms.size(), lmCoords.size());

            for (int k = 0; k < lms.size(); ++k) {
                QVERIFY(lmCoords.contains(lms.at(k).coordinate()));
            }
        }
    }

    m_manager->removeLandmarks(m_manager->landmarkIds());

    //TODO: more edge cases, async version of these tests
    QGeoCoordinate nearNorthPole(89.91,0);
    QLandmarkProximityFilter proximityFilter;
    proximityFilter.setCenter(nearNorthPole);
    proximityFilter.setRadius(11000);

    QVERIFY(doFetch(type, proximityFilter,&lms, QLandmarkManager::BadArgumentError));

    proximityFilter.setCenter(nearNorthPole);
    proximityFilter.setRadius(9000);
    QVERIFY(doFetch(type, proximityFilter,&lms, QLandmarkManager::NoError));

}

void tst_QLandmarkManager::filterLandmarksProximity_data() {
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::filterLandmarksProximityOrder()
{
    QFETCH(QString, type);
    QLandmark lm1;
    lm1.setName("LM1");
    lm1.setCoordinate(QGeoCoordinate(20,19));
    m_manager->saveLandmark(&lm1);

    QLandmark lm2;
    lm2.setName("LM2");
    lm2.setCoordinate(QGeoCoordinate(20,50));
    m_manager->saveLandmark(&lm2);

    QLandmark lm3;
    lm3.setName("LM3");
    lm3.setCoordinate(QGeoCoordinate(20, 30));
    m_manager->saveLandmark(&lm3);

    QLandmark lm4;
    lm4.setName("LM4");
    lm4.setCoordinate(QGeoCoordinate(5,20));
    m_manager->saveLandmark(&lm4);

    QLandmark lm5;
    lm5.setName("LM5");
    lm5.setCoordinate(QGeoCoordinate(80,20));
    m_manager->saveLandmark(&lm5);

    QLandmark lm6;
    lm6.setName("LM6");
    lm6.setCoordinate(QGeoCoordinate(60,20));
    m_manager->saveLandmark(&lm6);

    QLandmarkProximityFilter proximityFilter;
    proximityFilter.setCenter(QGeoCoordinate(20,20));
    QList<QLandmark> lms;
    QVERIFY(doFetch(type,proximityFilter, &lms, QLandmarkManager::NoError));
    QCOMPARE(lms.count(), 6);
    QCOMPARE(lms.at(0), lm1);
    QCOMPARE(lms.at(1), lm3);
    QCOMPARE(lms.at(2), lm4);
    QCOMPARE(lms.at(3), lm2);
    QCOMPARE(lms.at(4), lm6);
    QCOMPARE(lms.at(5), lm5);

    qreal radius = QGeoCoordinate(20,20).distanceTo(QGeoCoordinate(20,50));
    proximityFilter.setRadius(radius);
    QVERIFY(doFetch(type, proximityFilter,&lms,QLandmarkManager::NoError));

    QCOMPARE(lms.count(),4);
    QCOMPARE(lms.at(0), lm1);
    QCOMPARE(lms.at(1), lm3);
    QCOMPARE(lms.at(2), lm4);
    QCOMPARE(lms.at(3), lm2);

    //try a radius of less than -1
    proximityFilter.setRadius(-5);
    QVERIFY(doFetch(type, proximityFilter,&lms,QLandmarkManager::NoError));
    QCOMPARE(lms.count(), 6);
    QCOMPARE(lms.at(0), lm1);
    QCOMPARE(lms.at(1), lm3);
    QCOMPARE(lms.at(2), lm4);
    QCOMPARE(lms.at(3), lm2);
    QCOMPARE(lms.at(4), lm6);
    QCOMPARE(lms.at(5), lm5);

    //try a radius of 0
    proximityFilter.setCenter(QGeoCoordinate(20,30));
    proximityFilter.setRadius(0);
    QVERIFY(doFetch(type, proximityFilter,&lms,QLandmarkManager::NoError));
    QCOMPARE(lms.count(), 1);
    QCOMPARE(lms.at(0), lm3);

    //try a proximity filter with invalid center;
    proximityFilter.setCenter(QGeoCoordinate());
    proximityFilter.setRadius(5000);
    QVERIFY(doFetch(type, proximityFilter,&lms,QLandmarkManager::BadArgumentError));
    QCOMPARE(lms.count(), 0);

    //try proximity filter with latitude  Nan Value
    proximityFilter.setCenter(QGeoCoordinate(qQNaN(), 50));
    proximityFilter.setRadius(5000);
    QVERIFY(doFetch(type, proximityFilter,&lms,QLandmarkManager::BadArgumentError));
    QCOMPARE(lms.count(), 0);

    //try proximity filter with longitude a  Nan Value
    proximityFilter.setCenter(QGeoCoordinate(50,qQNaN()));
    proximityFilter.setRadius(5000);
    QVERIFY(doFetch(type, proximityFilter,&lms,QLandmarkManager::BadArgumentError));
    QCOMPARE(lms.count(), 0);

    //try a proximity filter with an out of range latitude
    proximityFilter.setCenter(QGeoCoordinate(90, 10));
    proximityFilter.setRadius(5000);
    QVERIFY(doFetch(type, proximityFilter,&lms,QLandmarkManager::BadArgumentError));
    QCOMPARE(lms.count(), 0);

    proximityFilter.setCenter(QGeoCoordinate(150, 10));
    proximityFilter.setRadius(5000);
    QVERIFY(doFetch(type, proximityFilter,&lms,QLandmarkManager::BadArgumentError));
    QCOMPARE(lms.count(), 0);

    //try a proximity filter with an out of range longitude
    proximityFilter.setCenter(QGeoCoordinate(-12, 180.1));
    proximityFilter.setRadius(5000);
    QVERIFY(doFetch(type, proximityFilter,&lms,QLandmarkManager::BadArgumentError));
    QCOMPARE(lms.count(), 0);

}

void tst_QLandmarkManager::filterLandmarksProximityOrder_data() {
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::filterLandmarksCategory() {
    QFETCH(QString, type);
    QLandmarkCategory cat1;
    cat1.setName("CAT1");
    QVERIFY(m_manager->saveCategory(&cat1));

    QLandmarkCategory cat2;
    cat2.setName("CAT2");
    QVERIFY(m_manager->saveCategory(&cat2));

    QLandmarkCategory cat3;
    cat3.setName("CAT3");
    QVERIFY(m_manager->saveCategory(&cat3));

    QLandmark lm1;
    lm1.setName("LM1");
    lm1.addCategoryId(cat1.categoryId());
    QVERIFY(m_manager->saveLandmark(&lm1));

    QLandmark lm2;
    lm2.setName("LM2");
    lm2.addCategoryId(cat1.categoryId());
    lm2.addCategoryId(cat2.categoryId());
    QVERIFY(m_manager->saveLandmark(&lm2));

    QLandmark lm3;
    lm3.setName("LM3");
    lm3.addCategoryId(cat1.categoryId());
    lm3.addCategoryId(cat2.categoryId());
    lm3.addCategoryId(cat3.categoryId());
    QVERIFY(m_manager->saveLandmark(&lm3));

    QLandmark lm4;
    lm4.setName("LM4");
    lm4.addCategoryId(cat2.categoryId());
    lm4.addCategoryId(cat3.categoryId());
    QVERIFY(m_manager->saveLandmark(&lm4));

    QLandmark lm5;
    lm5.setName("LM5");
    lm5.addCategoryId(cat3.categoryId());
    QVERIFY(m_manager->saveLandmark(&lm5));

    QLandmarkCategoryFilter filter(cat2.categoryId());

    QList<QLandmark> lms;
    QVERIFY(doFetch(type,filter,&lms, QLandmarkManager::NoError));

    QCOMPARE(lms.size(), 3);

    QSet<QString> names;
    for (int i = 0; i < lms.size(); ++i) {
        names.insert(lms.at(i).name());
    }

    QSet<QString> expectedNames;
    expectedNames.insert("LM2");
    expectedNames.insert("LM3");
    expectedNames.insert("LM4");

    QCOMPARE(names, expectedNames);

    //try a default category id
    QLandmarkCategoryId idNotExist;
    filter.setCategoryId(idNotExist);
    QVERIFY(doFetch(type,filter, &lms, QLandmarkManager::DoesNotExistError));

   //try a category with an empty local id
    QLandmarkCategoryId idNotExist2;
    idNotExist2.setManagerUri(m_manager->managerUri());
    filter.setCategoryId(idNotExist2);
    QVERIFY(doFetch(type,filter, &lms, QLandmarkManager::DoesNotExistError));

    //try a category with a valid manager uri but local id that does not exist
    QLandmarkCategoryId idNotExist3;
    idNotExist3.setManagerUri(m_manager->managerUri());
    idNotExist3.setLocalId("100");
    filter.setCategoryId(idNotExist3);
    QVERIFY(doFetch(type,filter, &lms, QLandmarkManager::NoError));
}

void tst_QLandmarkManager::filterLandmarksCategory_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::filterLandmarksBox() {
    QFETCH(QString, type);
    QList<QGeoCoordinate> outBox;

    QList<QGeoCoordinate> inBox1;

    inBox1 << QGeoCoordinate(-5.0, -5.0);
    inBox1 << QGeoCoordinate(-5.0, 0.0);
    inBox1 << QGeoCoordinate(-5.0, 5.0);
    inBox1 << QGeoCoordinate(0.0, -5.0);
    inBox1 << QGeoCoordinate(0.0, 0.0);
    inBox1 << QGeoCoordinate(0.0, 5.0);
    inBox1 << QGeoCoordinate(5.0, -5.0);
    inBox1 << QGeoCoordinate(5.0, 0.0);
    inBox1 << QGeoCoordinate(5.0, 5.0);

    outBox << QGeoCoordinate(-5.0, -6.0);
    outBox << QGeoCoordinate(5.0, -6.0);
    outBox << QGeoCoordinate(-6.0, -5.0);
    outBox << QGeoCoordinate(6.0, -5.0);
    outBox << QGeoCoordinate(-6.0, 5.0);
    outBox << QGeoCoordinate(6.0, 5.0);
    outBox << QGeoCoordinate(-5.0, 6.0);
    outBox << QGeoCoordinate(5.0, 6.0);

    QList<QGeoCoordinate> inBox2;

    inBox2 << QGeoCoordinate(-5.0, 10.0);
    inBox2 << QGeoCoordinate(-5.0, 15.0);
    inBox2 << QGeoCoordinate(-5.0, 20.0);
    inBox2 << QGeoCoordinate(0.0, 10.0);
    inBox2 << QGeoCoordinate(0.0, 15.0);
    inBox2 << QGeoCoordinate(0.0, 20.0);
    inBox2 << QGeoCoordinate(5.0, 10.0);
    inBox2 << QGeoCoordinate(5.0, 15.0);
    inBox2 << QGeoCoordinate(5.0, 20.0);

    outBox << QGeoCoordinate(-5.0, 9.0);
    outBox << QGeoCoordinate(5.0,  9.0);
    outBox << QGeoCoordinate(-6.0, 10.0);
    outBox << QGeoCoordinate(6.0,  10.0);
    outBox << QGeoCoordinate(-6.0, 20.0);
    outBox << QGeoCoordinate(6.0, 20.0);
    outBox << QGeoCoordinate(-5.0, 21.0);
    outBox << QGeoCoordinate(5.0, 21.0);

    QList<QGeoCoordinate> inBox3;

    inBox3 << QGeoCoordinate(10.0, -5.0);
    inBox3 << QGeoCoordinate(10.0, 0.0);
    inBox3 << QGeoCoordinate(10.0, 5.0);
    inBox3 << QGeoCoordinate(15.0, -5.0);
    inBox3 << QGeoCoordinate(15.0, 0.0);
    inBox3 << QGeoCoordinate(15.0, 5.0);
    inBox3 << QGeoCoordinate(20.0, -5.0);
    inBox3 << QGeoCoordinate(20.0, 0.0);
    inBox3 << QGeoCoordinate(20.0, 5.0);

    outBox << QGeoCoordinate(10.0, -6.0);
    outBox << QGeoCoordinate(20.0, -6.0);
    outBox << QGeoCoordinate(9.0, -5.0);
    outBox << QGeoCoordinate(21.0, -5.0);
    outBox << QGeoCoordinate(9.0, 5.0);
    outBox << QGeoCoordinate(21.0, 5.0);
    outBox << QGeoCoordinate(10.0, 6.0);
    outBox << QGeoCoordinate(20.0, 6.0);

    QList<QGeoCoordinate> inBox4;

    inBox4 << QGeoCoordinate(10.0, 10.0);
    inBox4 << QGeoCoordinate(10.0, 15.0);
    inBox4 << QGeoCoordinate(10.0, 20.0);
    inBox4 << QGeoCoordinate(15.0, 10.0);
    inBox4 << QGeoCoordinate(15.0, 15.0);
    inBox4 << QGeoCoordinate(15.0, 20.0);
    inBox4 << QGeoCoordinate(20.0, 10.0);
    inBox4 << QGeoCoordinate(20.0, 15.0);
    inBox4 << QGeoCoordinate(20.0, 20.0);

    outBox << QGeoCoordinate(10.0, 9.0);
    outBox << QGeoCoordinate(20.0, 9.0);
    outBox << QGeoCoordinate(9.0, 10.0);
    outBox << QGeoCoordinate(21.0, 10.0);
    outBox << QGeoCoordinate(9.0, 20.0);
    outBox << QGeoCoordinate(21.0, 20.0);
    outBox << QGeoCoordinate(10.0, 21.0);
    outBox << QGeoCoordinate(20.0, 21.0);

    QList<QGeoCoordinate> inBox5;

    inBox5 << QGeoCoordinate(-5.0, 175.0);
    inBox5 << QGeoCoordinate(-5.0, 180.0);
    inBox5 << QGeoCoordinate(-5.0, -175.0);
    inBox5 << QGeoCoordinate(0.0, 175.0);
    inBox5 << QGeoCoordinate(0.0, 180.0);
    inBox5 << QGeoCoordinate(0.0, -175.0);
    inBox5 << QGeoCoordinate(5.0, 175.0);
    inBox5 << QGeoCoordinate(5.0, 180.0);
    inBox5 << QGeoCoordinate(5.0, -175.0);

    outBox << QGeoCoordinate(-6.0, 175.0);
    outBox << QGeoCoordinate(-6.0, -175.0);
    outBox << QGeoCoordinate(-5.0, 174.0);
    outBox << QGeoCoordinate(-5.0, -174.0);
    outBox << QGeoCoordinate(5.0, 174.0);
    outBox << QGeoCoordinate(5.0, -174.0);
    outBox << QGeoCoordinate(6.0, 175.0);
    outBox << QGeoCoordinate(6.0, -175.0);

    QList<QGeoCoordinate> coords = outBox;
    coords.append(inBox1);
    coords.append(inBox2);
    coords.append(inBox3);
    coords.append(inBox4);
    coords.append(inBox5);

    for (int i = 0; i < coords.size(); ++i) {
        QLandmark lm;
        lm.setCoordinate(coords.at(i));
        QVERIFY(m_manager->saveLandmark(&lm));
    }

    QLandmarkBoxFilter filter1(QGeoCoordinate(5.0, -5.0), QGeoCoordinate(-5.0, 5.0));
    QList<QLandmark> lms1;
    QVERIFY(doFetch(type, filter1, &lms1, QLandmarkManager::NoError));

    QCOMPARE(lms1.size(), inBox1.size());

    QSet<QString> testSet1;
    for (int i = 0; i < lms1.size(); ++i)
        testSet1.insert(lms1.at(i).coordinate().toString());

    QSet<QString> inBoxSet1;
    for (int i = 0; i < inBox1.size(); ++i)
        inBoxSet1.insert(inBox1.at(i).toString());

    QCOMPARE(testSet1, inBoxSet1);

    QLandmarkBoxFilter filter2(QGeoCoordinate(5.0, 10.0), QGeoCoordinate(-5.0, 20.0));
    QList<QLandmark> lms2;
    QVERIFY(doFetch(type, filter2, &lms2, QLandmarkManager::NoError));
    QCOMPARE(lms2.size(), inBox2.size());

    QSet<QString> testSet2;
    for (int i = 0; i < lms2.size(); ++i)
        testSet2.insert(lms2.at(i).coordinate().toString());

    QSet<QString> inBoxSet2;
    for (int i = 0; i < inBox2.size(); ++i)
        inBoxSet2.insert(inBox2.at(i).toString());

    QCOMPARE(testSet2, inBoxSet2);

    QLandmarkBoxFilter filter3(QGeoCoordinate(20.0, -5.0), QGeoCoordinate(10.0, 5.0));
    QList<QLandmark> lms3;
    QVERIFY(doFetch(type, filter3, &lms3, QLandmarkManager::NoError));

    QCOMPARE(lms3.size(), inBox3.size());

    QSet<QString> testSet3;
    for (int i = 0; i < lms3.size(); ++i)
        testSet3.insert(lms3.at(i).coordinate().toString());

    QSet<QString> inBoxSet3;
    for (int i = 0; i < inBox3.size(); ++i)
        inBoxSet3.insert(inBox3.at(i).toString());

    QCOMPARE(testSet3, inBoxSet3);

    QLandmarkBoxFilter filter4(QGeoCoordinate(20.0, 10.0), QGeoCoordinate(10.0, 20.0));
    QList<QLandmark> lms4;
    QVERIFY(doFetch(type, filter4, &lms4, QLandmarkManager::NoError));
    QCOMPARE(lms4.size(), inBox4.size());

    QSet<QString> testSet4;
    for (int i = 0; i < lms4.size(); ++i)
        testSet4.insert(lms4.at(i).coordinate().toString());

    QSet<QString> inBoxSet4;
    for (int i = 0; i < inBox4.size(); ++i)
        inBoxSet4.insert(inBox4.at(i).toString());

    QCOMPARE(testSet4, inBoxSet4);

    QLandmarkBoxFilter filter5(QGeoCoordinate(5.0, 175.0), QGeoCoordinate(-5.0, -175.0));
    QList<QLandmark> lms5;
    QVERIFY(doFetch(type, filter5, &lms5, QLandmarkManager::NoError));
    QCOMPARE(lms5.size(), inBox5.size());

    QSet<QString> testSet5;
    for (int i = 0; i < lms5.size(); ++i)
        testSet5.insert(lms5.at(i).coordinate().toString());

    QSet<QString> inBoxSet5;
    for (int i = 0; i < inBox5.size(); ++i)
        inBoxSet5.insert(inBox5.at(i).toString());

    QCOMPARE(testSet5, inBoxSet5);

    //TODO: try different sets of invalid coordinates for top right and bottom left;
    QGeoBoundingBox box;
    QGeoCoordinate topLeft;
    topLeft.setLatitude(20);
    topLeft.setLongitude(20);

    QGeoCoordinate bottomRight;
    bottomRight.setLatitude(50);
    bottomRight.setLongitude(30);

    box.setTopLeft(topLeft);
    box.setBottomRight(bottomRight);

    QList<QLandmark> lms;
    QLandmarkBoxFilter filter;
    filter.setBoundingBox(box);
    QVERIFY(doFetch(type, filter,&lms, QLandmarkManager::BadArgumentError));

    //try an invalid coordinate for one of the corners
    topLeft.setLatitude(qQNaN());
    box.setTopLeft(topLeft);

    filter.setBoundingBox(box);
    QVERIFY(doFetch(type, filter,&lms, QLandmarkManager::BadArgumentError));

    //TODO: more invalid types of boxes
}

void tst_QLandmarkManager::filterLandmarksBox_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::filterLandmarksIntersection() {
    QFETCH(QString, type);
    QLandmarkCategory cat1;
    cat1.setName("CAT1");
    QVERIFY(m_manager->saveCategory(&cat1));

    QLandmarkCategory cat2;
    cat2.setName("CAT2");
    QVERIFY(m_manager->saveCategory(&cat2));

    QLandmarkCategory cat3;
    cat3.setName("CAT3");
    QVERIFY(m_manager->saveCategory(&cat3));

    QList<QLandmarkCategoryId> ids;
    ids << cat1.categoryId();
    ids << cat2.categoryId();
    ids << cat3.categoryId();

    QList<QString> names;
    names << "LM1";
    names << "LM2";
    names << "LM3";

    QList<QGeoCoordinate> coords;
    coords << QGeoCoordinate(0.0, 0.0);
    coords << QGeoCoordinate(25.0, 25.0);
    coords << QGeoCoordinate(50.0, 50.0);

    QList<QLandmark> lmPool;

    for (int i = 0; i < ids.size(); ++i) {
        for (int j = 0; j < names.size(); ++j) {
            for (int k = 0; k < coords.size(); ++k) {
                QLandmark lm;
                lm.addCategoryId(ids.at(i));
                lm.setName(names.at(j));
                lm.setCoordinate(coords.at(k));
                QVERIFY(m_manager->saveLandmark(&lm));
                lmPool << lm;
            }
        }
    }

    QLandmarkCategoryFilter f1(cat2.categoryId());
    QLandmarkNameFilter f2("LM2");
    QLandmarkProximityFilter f3(QGeoCoordinate(25.0, 25.0), 5.0);

    QLandmarkIntersectionFilter filter;
    filter << f1 << f2 << f3;

    QList<QLandmark> lms;
    QVERIFY(doFetch(type, filter, &lms,QLandmarkManager::NoError));

    QCOMPARE(lms.size(), 1);

    QSet<QString> idSet;
    for (int i = 0; i < lms.size(); ++i)
        idSet.insert(lms.at(i).landmarkId().localId());

    for (int i = 0; i < lmPool.size(); ++i) {
        QLandmark lm = lmPool.at(i);
        if ((lm.categoryIds().at(0) == cat2.categoryId())
            && (lm.name() == "LM2")
            && (lm.coordinate() == QGeoCoordinate(25.0, 25.0))) {
            QCOMPARE(idSet.contains(lm.landmarkId().localId()), true);
        } else {
            QCOMPARE(idSet.contains(lm.landmarkId().localId()), false);
        }
    }

    filter.remove(f2);

    QVERIFY(doFetch(type, filter, &lms,QLandmarkManager::NoError));
    QCOMPARE(lms.size(), 3);

    idSet.clear();
    for (int i = 0; i < lms.size(); ++i)
        idSet.insert(lms.at(i).landmarkId().localId());

    for (int i = 0; i < lmPool.size(); ++i) {
        QLandmark lm = lmPool.at(i);
        if ((lm.categoryIds().at(0) == cat2.categoryId())
            && (lm.coordinate() == QGeoCoordinate(25.0, 25.0))) {
            QCOMPARE(idSet.contains(lm.landmarkId().localId()), true);
        } else {
            QCOMPARE(idSet.contains(lm.landmarkId().localId()), false);
        }
    }

    filter.prepend(f2);

    QVERIFY(doFetch(type, filter, &lms,QLandmarkManager::NoError));
    QCOMPARE(lms.size(), 1);

    idSet.clear();
    for (int i = 0; i < lms.size(); ++i)
        idSet.insert(lms.at(i).landmarkId().localId());

    for (int i = 0; i < lmPool.size(); ++i) {
        QLandmark lm = lmPool.at(i);
        if ((lm.categoryIds().at(0) == cat2.categoryId())
            && (lm.name() == "LM2")
            && (lm.coordinate() == QGeoCoordinate(25.0, 25.0))) {
            QCOMPARE(idSet.contains(lm.landmarkId().localId()), true);
        } else {
            QCOMPARE(idSet.contains(lm.landmarkId().localId()), false);
        }
    }

    m_manager->removeLandmarks(m_manager->landmarkIds());

    QCOMPARE(m_manager->landmarkIds().count(), 0);
    QLandmarkCategory cat4;
    cat4.setName("CAT4");
    m_manager->saveCategory(&cat4);

    QLandmark lm1; //outside
    lm1.setName("LM1");
    lm1.setCoordinate(QGeoCoordinate(50,55.1));
    lm1.addCategoryId(cat1.categoryId());
    m_manager->saveLandmark(&lm1);

    QLandmark lm2;
    lm2.setName("LM2");
    lm2.setCoordinate(QGeoCoordinate(50,55));
    lm2.addCategoryId(cat3.categoryId());
    m_manager->saveLandmark(&lm2);

    QLandmark lm3;
    lm3.setName("LM3");
    lm3.setCoordinate(QGeoCoordinate(53,50));
    lm3.addCategoryId(cat1.categoryId());
    m_manager->saveLandmark(&lm3);

    QLandmark lm4;//outside
    lm4.setName("LM4");
    lm4.setCoordinate(QGeoCoordinate(53.23,50));
    m_manager->saveLandmark(&lm4);

    QLandmark lm5;
    lm5.setName("LM5");
    lm5.setCoordinate(QGeoCoordinate(51,51));
    lm5.addCategoryId(cat2.categoryId());
    lm5.addCategoryId(cat1.categoryId());
    m_manager->saveLandmark(&lm5);

    QLandmark lm6;
    lm6.setName("LM6");
    lm6.setCoordinate(QGeoCoordinate(52,48));
    lm6.addCategoryId(cat2.categoryId());
    m_manager->saveLandmark(&lm6);

    QLandmark lm7;//outside
    lm7.setName("LM7");
    lm7.setCoordinate(QGeoCoordinate(52.66, 47));
    m_manager->saveLandmark(&lm7);

    QLandmark lm8;//outside
    lm8.setName("LM8");
    lm8.setCoordinate(QGeoCoordinate(46,50));
    m_manager->saveLandmark(&lm8);

    QLandmark lm9;
    lm9.setName("LM9");
    lm9.setCoordinate(QGeoCoordinate(51, 48.5));
    lm9.addCategoryId(cat1.categoryId());
    lm9.addCategoryId(cat2.categoryId());
    m_manager->saveLandmark(&lm9);

    QLandmark lm10;
    lm10.setName("LM10");
    lm10.setCoordinate(QGeoCoordinate(49,49));
    lm10.addCategoryId(cat1.categoryId());
    m_manager->saveLandmark(&lm10);

    QLandmark lm11;
    lm11.setName("LM11");
    lm11.setCoordinate(QGeoCoordinate(48,51));
    m_manager->saveLandmark(&lm11);

    QLandmark lm12;
    lm12.setName("LM12");
    lm12.setCoordinate(QGeoCoordinate(48,53.83));
    lm12.addCategoryId(cat3.categoryId());
    m_manager->saveLandmark(&lm12);

    QLandmarkProximityFilter proximityFilter;
    proximityFilter.setCenter(QGeoCoordinate(50,50));
    proximityFilter.setRadius(-1);
    QVERIFY(doFetch(type,proximityFilter, &lms));
    QCOMPARE(lms.count(), 12);
    QCOMPARE(lms.at(0),lm5);
    QCOMPARE(lms.at(1), lm10);
    QCOMPARE(lms.at(2), lm9);
    QCOMPARE(lms.at(3), lm11);
    QCOMPARE(lms.at(4), lm6);
    QCOMPARE(lms.at(5), lm3);
    QCOMPARE(lms.at(6), lm12);
    QCOMPARE(lms.at(7), lm2);
    QCOMPARE(lms.at(8), lm4);
    QCOMPARE(lms.at(9), lm7);
    QCOMPARE(lms.at(10), lm1);
    QCOMPARE(lms.at(11), lm8);

    proximityFilter.setRadius(358000);
    QVERIFY(doFetch(type,proximityFilter, &lms));
    QCOMPARE(lms.count(), 8);
    QCOMPARE(lms.at(0),lm5);
    QCOMPARE(lms.at(1), lm10);
    QCOMPARE(lms.at(2), lm9);
    QCOMPARE(lms.at(3), lm11);
    QCOMPARE(lms.at(4), lm6);
    QCOMPARE(lms.at(5), lm3);
    QCOMPARE(lms.at(6), lm12);
    QCOMPARE(lms.at(7), lm2);

    QLandmarkIntersectionFilter intersectionFilter;
    proximityFilter.setRadius(-1);
    intersectionFilter.append(proximityFilter);
    QVERIFY(doFetch(type,proximityFilter, &lms));
    QCOMPARE(lms.count(), 12);
    QCOMPARE(lms.at(0),lm5);
    QCOMPARE(lms.at(1), lm10);
    QCOMPARE(lms.at(2), lm9);
    QCOMPARE(lms.at(3), lm11);
    QCOMPARE(lms.at(4), lm6);
    QCOMPARE(lms.at(5), lm3);
    QCOMPARE(lms.at(6), lm12);
    QCOMPARE(lms.at(7), lm2);
    QCOMPARE(lms.at(8), lm4);
    QCOMPARE(lms.at(9), lm7);
    QCOMPARE(lms.at(10), lm1);
    QCOMPARE(lms.at(11), lm8);

    intersectionFilter.clear();
    proximityFilter.setRadius(358000);
    intersectionFilter.append(proximityFilter);
    QVERIFY(doFetch(type,intersectionFilter, &lms));
    QCOMPARE(lms.count(), 8);
    QCOMPARE(lms.at(0),lm5);
    QCOMPARE(lms.at(1), lm10);
    QCOMPARE(lms.at(2), lm9);
    QCOMPARE(lms.at(3), lm11);
    QCOMPARE(lms.at(4), lm6);
    QCOMPARE(lms.at(5), lm3);
    QCOMPARE(lms.at(6), lm12);
    QCOMPARE(lms.at(7), lm2);

    QLandmarkCategoryFilter cat1Filter;
    cat1Filter.setCategoryId(cat1.categoryId());

    QLandmarkCategoryFilter cat2Filter;
    cat2Filter.setCategoryId(cat2.categoryId());

    QLandmarkCategoryFilter cat3Filter;
    cat3Filter.setCategoryId(cat3.categoryId());

    //try proximity and a catgegory
    intersectionFilter.clear();
    intersectionFilter.append(proximityFilter);
    intersectionFilter.append(cat1Filter);
    QVERIFY(doFetch(type,intersectionFilter, &lms));
    QCOMPARE(lms.count(), 4);
    QCOMPARE(lms.at(0), lm5);
    QCOMPARE(lms.at(1), lm10);
    QCOMPARE(lms.at(2), lm9);
    QCOMPARE(lms.at(3), lm3);

    //try proximity and a different category
    intersectionFilter.clear();
    intersectionFilter.append(proximityFilter);
    intersectionFilter.append(cat2Filter);
    QVERIFY(doFetch(type,intersectionFilter, &lms));
    QCOMPARE(lms.count(),3);
    QCOMPARE(lms.at(0), lm5);
    QCOMPARE(lms.at(1), lm9);
    QCOMPARE(lms.at(2), lm6);

    //try a proximity and two categories together
    intersectionFilter.clear();
    intersectionFilter.append(cat1Filter);
    intersectionFilter.append(proximityFilter);
    intersectionFilter.append(cat2Filter);
    QVERIFY(doFetch(type,intersectionFilter, &lms));
    QCOMPARE(lms.count(),2);
    QCOMPARE(lms.at(0), lm5);
    QCOMPARE(lms.at(1), lm9);

    //try a proximity but with two categories t    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
    intersectionFilter.clear();
    intersectionFilter.append(cat1Filter);
    intersectionFilter.append(proximityFilter);
    intersectionFilter.append(cat3Filter);
    QVERIFY(doFetch(type,intersectionFilter, &lms));
    QCOMPARE(lms.count(),0);

    //try with a union of two categories which have no overlap
    QLandmarkUnionFilter unionFilter;
    unionFilter.append(cat1Filter);
    unionFilter.append(cat3Filter);
    intersectionFilter.clear();
    intersectionFilter.append(unionFilter);
    intersectionFilter.append(proximityFilter);
    QVERIFY(doFetch(type,intersectionFilter, &lms));
    QCOMPARE(lms.count(), 6);
    QCOMPARE(lms.at(0),lm5);
    QCOMPARE(lms.at(1), lm10);
    QCOMPARE(lms.at(2), lm9);
    QCOMPARE(lms.at(3), lm3);
    QCOMPARE(lms.at(4), lm12);
    QCOMPARE(lms.at(5), lm2);

    //try a union of two categories which do have overlap
    unionFilter.clear();
    unionFilter.append(cat2Filter);
    unionFilter.append(cat1Filter);
    intersectionFilter.clear();
    intersectionFilter.append(proximityFilter);
    intersectionFilter.append(unionFilter);
    QVERIFY(doFetch(type,intersectionFilter, &lms));
    QCOMPARE(lms.count(), 5);
    QCOMPARE(lms.at(0), lm5);
    QCOMPARE(lms.at(1), lm10);
    QCOMPARE(lms.at(2), lm9);
    QCOMPARE(lms.at(3), lm6);
    QCOMPARE(lms.at(4), lm3);

    //try an intersection filter categories but proximity doesn't have
    //landmarks in its region
    intersectionFilter.clear();
    intersectionFilter.append(cat2Filter);
    proximityFilter.setCenter(QGeoCoordinate(-70,-70));
    proximityFilter.setRadius(100000);
    intersectionFilter.append(proximityFilter);
    QVERIFY(doFetch(type,intersectionFilter, &lms));
    QCOMPARE(lms.count(), 0);

    //don't use a radius with the proximityFilter
    proximityFilter.setCenter(QGeoCoordinate(50,50));
    proximityFilter.setRadius(-1);
    intersectionFilter.clear();
    intersectionFilter.append(cat1Filter);
    intersectionFilter.append(proximityFilter);
    QVERIFY(doFetch(type,intersectionFilter, &lms));
    QCOMPARE(lms.count(), 5);
    QCOMPARE(lms.at(0), lm5);
    QCOMPARE(lms.at(1), lm10);
    QCOMPARE(lms.at(2), lm9);
    QCOMPARE(lms.at(3), lm3);
    QCOMPARE(lms.at(4), lm1);
}

void tst_QLandmarkManager::filterLandmarksIntersection_data() {
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::filterLandmarksMultipleBox()
{
    QFETCH(QString, type);
    QLandmarkBoxFilter boxFilter1(QGeoCoordinate(20,10), QGeoCoordinate(10,20));
    QLandmarkBoxFilter boxFilter2(QGeoCoordinate(20,15), QGeoCoordinate(10,25));
    QLandmarkBoxFilter boxFilter3(QGeoCoordinate(15,12.5), QGeoCoordinate(5,22.5));

    QLandmark lm1;
    lm1.setName("LM1");
    lm1.setCoordinate(QGeoCoordinate(17.5, 12.5));
    QVERIFY(m_manager->saveLandmark(&lm1));

    QLandmark lm2;
    lm2.setName("LM2");
    lm2.setCoordinate(QGeoCoordinate(17.5, 17.5));
    QVERIFY(m_manager->saveLandmark(&lm2));

    QLandmark lm3;
    lm3.setName("LM3");
    lm3.setCoordinate(QGeoCoordinate(17.5, 22.5));
    QVERIFY(m_manager->saveLandmark(&lm3));

    QLandmark lm4;
    lm4.setName("LM4");
    lm4.setCoordinate(QGeoCoordinate(12.5, 13.25));
    QVERIFY(m_manager->saveLandmark(&lm4));

    QLandmark lm5;
    lm5.setName("LM5");
    lm5.setCoordinate(QGeoCoordinate(12.5, 17.5));
    QVERIFY(m_manager->saveLandmark(&lm5));

    QLandmark lm6;
    lm6.setName("LM6");
    lm6.setCoordinate(QGeoCoordinate(12.5, 21.25));
    QVERIFY(m_manager->saveLandmark(&lm6));

    QLandmark lm7;
    lm7.setName("LM7");
    lm7.setCoordinate(QGeoCoordinate(5, 11.25));
    QVERIFY(m_manager->saveLandmark(&lm7));

    QLandmark lm8;
    lm8.setName("LM8");
    lm8.setCoordinate(QGeoCoordinate(7.5, 17.5));
    QVERIFY(m_manager->saveLandmark(&lm8));

    QLandmark lm9;
    lm9.setName("LM9");
    lm9.setCoordinate(QGeoCoordinate(5, 23.25));
    QVERIFY(m_manager->saveLandmark(&lm9));

    QLandmarkIntersectionFilter intersectionFilter;
    intersectionFilter.append(boxFilter1);
    intersectionFilter.append(boxFilter2);
    intersectionFilter.append(boxFilter3);

    //try all 3 box filters in an intersection
    QList<QLandmark> lms;
    QVERIFY(doFetch(type,intersectionFilter,&lms));
    QCOMPARE(lms.count(),1);
    QCOMPARE(lms.at(0).landmarkId(), lm5.landmarkId());

    //try combinations of 2 box filters
    intersectionFilter.clear();
    intersectionFilter.append(boxFilter1);
    intersectionFilter.append(boxFilter2);

    QVERIFY(doFetch(type,intersectionFilter,&lms));
    QCOMPARE(lms.count(), 2);
    QVERIFY(lms.contains(lm2));
    QVERIFY(lms.contains(lm5));

    intersectionFilter.clear();
    intersectionFilter.append(boxFilter2);
    intersectionFilter.append(boxFilter3);

    QVERIFY(doFetch(type,intersectionFilter,&lms));
    QCOMPARE(lms.count(), 2);
    QVERIFY(lms.contains(lm5));
    QVERIFY(lms.contains(lm6));

    intersectionFilter.clear();
    intersectionFilter.append(boxFilter1);
    intersectionFilter.append(boxFilter3);

    QVERIFY(doFetch(type,intersectionFilter,&lms));
    QCOMPARE(lms.count(), 2);
    QVERIFY(lms.contains(lm4));
    QVERIFY(lms.contains(lm5));

    //try intersection filter with only 1 box filter
    intersectionFilter.clear();
    intersectionFilter.append(boxFilter1);
    QVERIFY(doFetch(type,intersectionFilter,&lms));
    QCOMPARE(lms.count(), 4);
    QVERIFY(lms.contains(lm1));
    QVERIFY(lms.contains(lm2));
    QVERIFY(lms.contains(lm4));
    QVERIFY(lms.contains(lm5));
    QCOMPARE(lms,m_manager->landmarks(boxFilter1));

    intersectionFilter.clear();
    intersectionFilter.append(boxFilter2);
    QVERIFY(doFetch(type,intersectionFilter,&lms));
    QCOMPARE(lms.count(), 4);
    QVERIFY(lms.contains(lm2));
    QVERIFY(lms.contains(lm3));
    QVERIFY(lms.contains(lm5));
    QVERIFY(lms.contains(lm6));
    QCOMPARE(lms,m_manager->landmarks(boxFilter2));

    intersectionFilter.clear();
    intersectionFilter.append(boxFilter3);
    QVERIFY(doFetch(type,intersectionFilter,&lms));
    QCOMPARE(lms.count(), 4);
    QVERIFY(lms.contains(lm4));
    QVERIFY(lms.contains(lm5));
    QVERIFY(lms.contains(lm6));
    QVERIFY(lms.contains(lm8));
    QCOMPARE(lms,m_manager->landmarks(boxFilter3));

    //try different combinations of union filter
    //try union filter with all 3 box filters
    QLandmarkUnionFilter unionFilter;
    unionFilter.append(boxFilter1);
    unionFilter.append(boxFilter2);
    unionFilter.append(boxFilter3);

    QVERIFY(doFetch(type,unionFilter, &lms));
    QCOMPARE(lms.count(), 7);
    QVERIFY(lms.contains(lm1));
    QVERIFY(lms.contains(lm2));
    QVERIFY(lms.contains(lm3));
    QVERIFY(lms.contains(lm4));
    QVERIFY(lms.contains(lm5));
    QVERIFY(lms.contains(lm6));
    QVERIFY(lms.contains(lm8));

    //try combinations of 2 box filters
    unionFilter.clear();
    unionFilter.append(boxFilter1);
    unionFilter.append(boxFilter2);
    QVERIFY(doFetch(type,unionFilter, &lms));
    QCOMPARE(lms.count(), 6);
    QVERIFY(lms.contains(lm1));
    QVERIFY(lms.contains(lm2));
    QVERIFY(lms.contains(lm3));
    QVERIFY(lms.contains(lm4));
    QVERIFY(lms.contains(lm5));
    QVERIFY(lms.contains(lm6));

    unionFilter.clear();
    unionFilter.append(boxFilter2);
    unionFilter.append(boxFilter3);
    QVERIFY(doFetch(type,unionFilter, &lms));
    QCOMPARE(lms.count(), 6);
    QVERIFY(lms.contains(lm2));
    QVERIFY(lms.contains(lm3));
    QVERIFY(lms.contains(lm4));
    QVERIFY(lms.contains(lm5));
    QVERIFY(lms.contains(lm6));
    QVERIFY(lms.contains(lm8));

    unionFilter.clear();
    unionFilter.append(boxFilter1);
    unionFilter.append(boxFilter3);
    QVERIFY(doFetch(type,unionFilter, &lms));
    QCOMPARE(lms.count(), 6);
    QVERIFY(lms.contains(lm1));
    QVERIFY(lms.contains(lm2));
    QVERIFY(lms.contains(lm4));
    QVERIFY(lms.contains(lm5));
    QVERIFY(lms.contains(lm6));
    QVERIFY(lms.contains(lm8));

    //try a single filter in the union filter
    unionFilter.clear();
    unionFilter.append(boxFilter1);
    QVERIFY(doFetch(type, unionFilter, &lms));
    QCOMPARE(lms.count(), 4);
    QVERIFY(lms.contains(lm1));
    QVERIFY(lms.contains(lm2));
    QVERIFY(lms.contains(lm4));
    QVERIFY(lms.contains(lm5));
    QCOMPARE(lms,m_manager->landmarks(boxFilter1));

    unionFilter.clear();
    unionFilter.append(boxFilter2);
    QVERIFY(doFetch(type, unionFilter, &lms));
    QCOMPARE(lms.count(), 4);
    QVERIFY(lms.contains(lm2));
    QVERIFY(lms.contains(lm3));
    QVERIFY(lms.contains(lm5));
    QVERIFY(lms.contains(lm6));
    QCOMPARE(lms,m_manager->landmarks(boxFilter2));

    unionFilter.clear();
    unionFilter.append(boxFilter3);
    QVERIFY(doFetch(type, unionFilter, &lms));
    QCOMPARE(lms.count(), 4);
    QVERIFY(lms.contains(lm4));
    QVERIFY(lms.contains(lm5));
    QVERIFY(lms.contains(lm6));
    QVERIFY(lms.contains(lm8));
    QCOMPARE(lms,m_manager->landmarks(boxFilter3));

    //TODO: test cases with errors
}

void tst_QLandmarkManager::filterLandmarksMultipleBox_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::filterLandmarksUnion() {
    QFETCH(QString, type);
    QLandmarkCategory cat1;
    cat1.setName("CAT1");
    QVERIFY(m_manager->saveCategory(&cat1));

    QLandmarkCategory cat2;
    cat2.setName("CAT2");
    QVERIFY(m_manager->saveCategory(&cat2));

    QLandmarkCategory cat3;
    cat3.setName("CAT3");
    QVERIFY(m_manager->saveCategory(&cat3));

    QList<QLandmarkCategoryId> ids;
    ids << cat1.categoryId();
    ids << cat2.categoryId();
    ids << cat3.categoryId();

    QList<QString> names;
    names << "LM1";
    names << "LM2";
    names << "LM3";

    QList<QGeoCoordinate> coords;
    coords << QGeoCoordinate(0.0, 0.0);
    coords << QGeoCoordinate(25.0, 25.0);
    coords << QGeoCoordinate(50.0, 50.0);

    QList<QLandmark> lmPool;

    for (int i = 0; i < ids.size(); ++i) {
        for (int j = 0; j < names.size(); ++j) {
            for (int k = 0; k < coords.size(); ++k) {
                QLandmark lm;
                lm.addCategoryId(ids.at(i));
                lm.setName(names.at(j));
                lm.setCoordinate(coords.at(k));
                QVERIFY(m_manager->saveLandmark(&lm));
                lmPool << lm;
            }
        }
    }

    QLandmarkCategoryFilter f1(cat2.categoryId());
    QLandmarkNameFilter f2("LM2");
    QLandmarkProximityFilter f3(QGeoCoordinate(25.0, 25.0), 5.0);

    QLandmarkUnionFilter filter;
    filter << f1 << f2 << f3;

    QList<QLandmark> lms;
    QVERIFY(doFetch(type, filter, &lms, QLandmarkManager::NoError));

    QCOMPARE(lms.size(), 19);

    QSet<QString> idSet;
    for (int i = 0; i < lms.size(); ++i)
        idSet.insert(lms.at(i).landmarkId().localId());

    for (int i = 0; i < lmPool.size(); ++i) {
        QLandmark lm = lmPool.at(i);
        if ((lm.categoryIds().at(0) == cat2.categoryId())
            || (lm.name() == "LM2")
            || (lm.coordinate() == QGeoCoordinate(25.0, 25.0))) {
            QCOMPARE(idSet.contains(lm.landmarkId().localId()), true);
        } else {
            QCOMPARE(idSet.contains(lm.landmarkId().localId()), false);
        }
    }

    filter.remove(f2);

    QVERIFY(doFetch(type, filter, &lms, QLandmarkManager::NoError));

    QCOMPARE(lms.size(), 15);

    idSet.clear();
    for (int i = 0; i < lms.size(); ++i)
        idSet.insert(lms.at(i).landmarkId().localId());

    for (int i = 0; i < lmPool.size(); ++i) {
        QLandmark lm = lmPool.at(i);
        if ((lm.categoryIds().at(0) == cat2.categoryId())
            || (lm.coordinate() == QGeoCoordinate(25.0, 25.0))) {
            QCOMPARE(idSet.contains(lm.landmarkId().localId()), true);
        } else {
            QCOMPARE(idSet.contains(lm.landmarkId().localId()), false);
        }
    }

    filter.prepend(f2);

    QVERIFY(doFetch(type, filter, &lms, QLandmarkManager::NoError));

    QCOMPARE(lms.size(), 19);

    idSet.clear();
    for (int i = 0; i < lms.size(); ++i)
        idSet.insert(lms.at(i).landmarkId().localId());

    for (int i = 0; i < lmPool.size(); ++i) {
        QLandmark lm = lmPool.at(i);
        if ((lm.categoryIds().at(0) == cat2.categoryId())
            || (lm.name() == "LM2")
            || (lm.coordinate() == QGeoCoordinate(25.0, 25.0))) {
            QCOMPARE(idSet.contains(lm.landmarkId().localId()), true);
        } else {
            QCOMPARE(idSet.contains(lm.landmarkId().localId()), false);
        }
    }
}

void tst_QLandmarkManager::filterLandmarksUnion_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::filterAttribute() {
    QLandmark lm1;
    lm1.setName("Adelaide");
    lm1.setDescription("The description of adelaide");
    QVERIFY(m_manager->saveLandmark(&lm1));

    QLandmark lm2;
    lm2.setName("Adel");
    lm2.setDescription("The description of adel");
    QVERIFY(m_manager->saveLandmark(&lm2));

    QLandmark lm3;
    lm3.setName("Brisbane");
    lm3.setDescription("The chronicles of brisbane");
    QVERIFY(m_manager->saveLandmark(&lm3));

    QLandmark lm4;
    lm4.setName("Perth");
    lm4.setDescription("The summary of perth");
    QVERIFY(m_manager->saveLandmark(&lm4));

    QLandmark lm5;
    lm5.setName("Canberra");
    lm5.setDescription("The chronicles of canberra");
    QVERIFY(m_manager->saveLandmark(&lm5));

    QLandmark lm6;
    lm6.setName("Tinberra");
    lm6.setDescription("The chronicles of tinberra");
    QVERIFY(m_manager->saveLandmark(&lm6));

    QLandmark lm7;
    lm7.setName("Madelaide");
    lm7.setDescription("The summary of madelaide");
    QVERIFY(m_manager->saveLandmark(&lm7));

    QLandmark lm8;
    lm8.setName("Terran");
    lm8.setDescription("Summary of terran");
    QVERIFY(m_manager->saveLandmark(&lm8));

    QLandmark lm9;
    lm9.setName("ADEL");
    lm9.setDescription("The summary of ADEL");
    QVERIFY(m_manager->saveLandmark(&lm9));

    QList<QLandmark> lms;
    QFETCH(QString, type);

    //test starts with
    QLandmarkAttributeFilter attributeFilter;
    attributeFilter.setAttribute("name", "adel",QLandmarkFilter::MatchStartsWith);
    QVERIFY(doFetch(type,attributeFilter,&lms));
    QCOMPARE(lms.count(), 3);

    QCOMPARE(lms.at(0), lm1);
    QCOMPARE(lms.at(1), lm2);
    QCOMPARE(lms.at(2), lm9);

    //test contains
    attributeFilter.setAttribute("name", "err", QLandmarkFilter::MatchContains);
    QVERIFY(doFetch(type,attributeFilter,&lms));
    QCOMPARE(lms.count(),3);
    QCOMPARE(lms.at(0), lm5);
    QCOMPARE(lms.at(1), lm6);
    QCOMPARE(lms.at(2), lm8);

     //test ends with
    attributeFilter.setAttribute("name", "ra", QLandmarkFilter::MatchEndsWith);
    QVERIFY(doFetch(type,attributeFilter,&lms));
    QCOMPARE(lms.count(),2);
    QCOMPARE(lms.at(0), lm5);
    QCOMPARE(lms.at(1), lm6);

    //test fixed string
    attributeFilter.setAttribute("name", "adel", QLandmarkFilter::MatchFixedString);
    QVERIFY(doFetch(type,attributeFilter,&lms));
    QCOMPARE(lms.count(), 2);
    QCOMPARE(lms.at(0), lm2);
    QCOMPARE(lms.at(1), lm9);

    //test match exactly
    attributeFilter.setAttribute("name", "Adel", QLandmarkFilter::MatchExactly);
    QVERIFY(doFetch(type,attributeFilter,&lms));
    QCOMPARE(lms.count(), 1);
    QCOMPARE(lms.at(0), lm2);

    //try ANDing multiple criteria
    attributeFilter.setOperationType(QLandmarkAttributeFilter::AndOperation);
    attributeFilter.setAttribute("name", "adel", QLandmarkFilter::MatchStartsWith);
    attributeFilter.setAttribute("description", "descript", QLandmarkFilter::MatchContains);
    QVERIFY(doFetch(type,attributeFilter,&lms));
    QCOMPARE(lms.count(),2);
    QCOMPARE(lms.at(0), lm1);
    QCOMPARE(lms.at(1), lm2);

    //try ORing multiple criteria
    attributeFilter.setOperationType(QLandmarkAttributeFilter::OrOperation);
    attributeFilter.setAttribute("name", "adel", QLandmarkFilter::MatchFixedString);
    attributeFilter.setAttribute("description", "the summary", QLandmarkFilter::MatchStartsWith);
    QVERIFY(doFetch(type,attributeFilter,&lms));
    QCOMPARE(lms.count(), 4);

    QVERIFY(lms.contains(lm2));
    QVERIFY(lms.contains(lm4));
    QVERIFY(lms.contains(lm7));
    QVERIFY(lms.contains(lm9));

    //try an single empty qvariant for and and or
    //should return all landmarks since all landmark will the values
    attributeFilter.clearAttributes();
    attributeFilter.setOperationType(QLandmarkAttributeFilter::AndOperation);
    attributeFilter.setAttribute("street");
    QVERIFY(doFetch(type,attributeFilter,&lms));
    QCOMPARE(lms.count(), 9);

    attributeFilter.clearAttributes();
    attributeFilter.setOperationType(QLandmarkAttributeFilter::OrOperation);
    attributeFilter.setAttribute("street");
    QVERIFY(doFetch(type,attributeFilter,&lms));
    QCOMPARE(lms.count(), 9);

    //try  with an empty qvariant, AND operation with multiple attributes
    attributeFilter.clearAttributes();
    attributeFilter.setOperationType(QLandmarkAttributeFilter::AndOperation);
    attributeFilter.setAttribute("street");
    attributeFilter.setAttribute("name", "Adelaide");
    QVERIFY(doFetch(type,attributeFilter,&lms));
    QCOMPARE(lms.count(),1);

    //try to return with an empty qvariant, OR operation with multiple attribute
    attributeFilter.clearAttributes();
    attributeFilter.setOperationType(QLandmarkAttributeFilter::OrOperation);
    attributeFilter.setAttribute("street");
    attributeFilter.setAttribute("name", "Adelaide");
    QVERIFY(doFetch(type,attributeFilter,&lms));
    QCOMPARE(lms.count(),9);

    //try all empty qvariatns AND operation with multiple attributes
    attributeFilter.clearAttributes();
    attributeFilter.setOperationType(QLandmarkAttributeFilter::AndOperation);
    attributeFilter.setAttribute("street");
    attributeFilter.setAttribute("description");
    attributeFilter.setAttribute("country");
    QVERIFY(doFetch(type,attributeFilter,&lms));
    QCOMPARE(lms.count(), 9);

    //todo: try filtering with an empty qvariant.
}

void tst_QLandmarkManager::filterAttribute_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::sortLandmarksNull() {
    QLandmark lm1;
    lm1.setName("b");
    QVERIFY(m_manager->saveLandmark(&lm1));

    QLandmark lm2;
    lm2.setName("a");
    QVERIFY(m_manager->saveLandmark(&lm2));

    QLandmark lm3;
    lm3.setName("c");
    QVERIFY(m_manager->saveLandmark(&lm3));

    QList<QLandmark> expected;
    expected << lm1;
    expected << lm2;
    expected << lm3;

    QLandmarkFilter filter;
    QLandmarkSortOrder sortOrder;
    QList<QLandmarkSortOrder> sortOrders;

    QList<QLandmark> lms = m_manager->landmarks(filter);
    QCOMPARE(lms, expected);

    lms = m_manager->landmarks(filter, -1, 0, sortOrder);
    QCOMPARE(lms, expected);

    lms = m_manager->landmarks(filter, -1, 0, sortOrders);
    QCOMPARE(lms, expected);

    sortOrders << sortOrder;
    lms = m_manager->landmarks(filter, -1, 0, sortOrders);
    QCOMPARE(lms, expected);

    sortOrders << sortOrder;
    lms = m_manager->landmarks(filter, -1, 0, sortOrders);
    QCOMPARE(lms, expected);
}

void tst_QLandmarkManager::sortLandmarksName() {
    QLandmark lm1;
    lm1.setName("b");
    QVERIFY(m_manager->saveLandmark(&lm1));

    QLandmark lm2;
    lm2.setName("a");
    QVERIFY(m_manager->saveLandmark(&lm2));

    QLandmark lm3;
    lm3.setName("c");
    QVERIFY(m_manager->saveLandmark(&lm3));

    QList<QLandmark> expectedAscending;
    expectedAscending << lm2;
    expectedAscending << lm1;
    expectedAscending << lm3;

    QList<QLandmark> expectedDescending;
    expectedDescending << lm3;
    expectedDescending << lm1;
    expectedDescending << lm2;

    QLandmarkFilter filter;
    QLandmarkNameSort sortAscending(Qt::AscendingOrder);

    QList<QLandmark> lms = m_manager->landmarks(filter, -1, 0, sortAscending);
    QCOMPARE(lms, expectedAscending);

    QLandmarkNameSort sortDescending(Qt::DescendingOrder);

    lms = m_manager->landmarks(filter, -1, 0, sortDescending);
    QCOMPARE(lms, expectedDescending);
}

void tst_QLandmarkManager::sortLandmarksNameAsync() {
    QLandmark lm1;
    lm1.setName("b");
    QVERIFY(m_manager->saveLandmark(&lm1));

    QLandmark lm2;
    lm2.setName("a");
    QVERIFY(m_manager->saveLandmark(&lm2));

    QLandmark lm3;
    lm3.setName("c");
    QVERIFY(m_manager->saveLandmark(&lm3));

    QLandmark lm4;
    lm4.setName("C");
    QVERIFY(m_manager->saveLandmark(&lm4));

    QLandmark lm5;
    lm5.setName("A");
    QVERIFY(m_manager->saveLandmark(&lm5));

    QLandmark lm6;
    lm6.setName("B");
    QVERIFY(m_manager->saveLandmark(&lm6));

    QList<QLandmark> expectedAscending;
    expectedAscending << lm2;
    expectedAscending << lm5;
    expectedAscending << lm1;
    expectedAscending << lm6;
    expectedAscending << lm3;
    expectedAscending << lm4;

    QList<QLandmark> expectedDescending;
    expectedDescending << lm3;
    expectedDescending << lm4;
    expectedDescending << lm1;
    expectedDescending << lm6;
    expectedDescending << lm2;
    expectedDescending << lm5;

    //test case insensitive ascending order
    QLandmarkFilter filter;
    QLandmarkNameSort sortAscending(Qt::AscendingOrder);
    QLandmarkFetchRequest fetchRequest(m_manager);
    fetchRequest.setSorting(sortAscending);
    QSignalSpy spy(&fetchRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
    fetchRequest.start();

    QVERIFY(waitForAsync(spy,&fetchRequest));
    QList<QLandmark> lms = fetchRequest.landmarks();
    QVERIFY(checkIdFetchRequest(lms,filter, sortAscending));

    QCOMPARE(lms, expectedAscending);

    //test case insensitive descending order
    QLandmarkNameSort sortDescending(Qt::DescendingOrder);
    fetchRequest.setSorting(sortDescending);
    fetchRequest.start();

    QVERIFY(waitForAsync(spy, &fetchRequest));
    lms = fetchRequest.landmarks();
    QVERIFY(checkIdFetchRequest(lms, filter, sortDescending));

    QCOMPARE(lms, expectedDescending);

    //test case sensitive ascending order
    expectedAscending.clear();
    expectedAscending << lm5;
    expectedAscending << lm6;
    expectedAscending << lm4;
    expectedAscending << lm2;
    expectedAscending << lm1;
    expectedAscending << lm3;

    sortAscending.setCaseSensitivity(Qt::CaseSensitive);
    fetchRequest.setSorting(sortAscending);
    fetchRequest.start();

    QVERIFY(waitForAsync(spy, &fetchRequest));
    lms = fetchRequest.landmarks();
    QVERIFY(checkIdFetchRequest(lms,filter,sortAscending));

    QCOMPARE(lms, expectedAscending);

    //test case sensitive descending order
    expectedDescending.clear();
    expectedDescending << lm3;
    expectedDescending << lm1;
    expectedDescending << lm2;
    expectedDescending << lm4;
    expectedDescending << lm6;
    expectedDescending << lm5;

    sortDescending.setCaseSensitivity(Qt::CaseSensitive);
    fetchRequest.setSorting(sortDescending);
    fetchRequest.start();

    QVERIFY(waitForAsync(spy, &fetchRequest));
    lms = fetchRequest.landmarks();
    QVERIFY(checkIdFetchRequest(lms,filter,sortDescending));

    QCOMPARE(lms, expectedDescending);
}

void tst_QLandmarkManager::importGpx() {
    QSignalSpy spyAdd(m_manager, SIGNAL(landmarksAdded(QList<QLandmarkId>)));
    QSignalSpy spyChange(m_manager,SIGNAL(landmarksChanged(QList<QLandmarkId>)));
    QSignalSpy spyRemove(m_manager,SIGNAL(landmarksRemoved(QList<QLandmarkId>)));

    QLandmarkImportRequest importRequest(m_manager);
    QSignalSpy spy(&importRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));

    QLandmarkCategory cat1;
    cat1.setName("CAT1");
    QVERIFY(m_manager->saveCategory(&cat1));

    QLandmarkCategory cat2;
    cat2.setName("CAT2");
    QVERIFY(m_manager->saveCategory(&cat2));

    QLandmarkCategory cat3;
    cat3.setName("CAT3");
    QVERIFY(m_manager->saveCategory(&cat3));
    QVERIFY(m_manager->removeCategory(cat3.categoryId()));

    QFETCH(QString, type);
    if (type == "sync")  {
        QVERIFY(!m_manager->importLandmarks(NULL, QLandmarkManager::Gpx)); //no iodevice set
        QCOMPARE(m_manager->error(), QLandmarkManager::BadArgumentError);

        QFile *noPermFile = new QFile("nopermfile");
        noPermFile->open(QIODevice::WriteOnly);
        noPermFile->setPermissions(QFile::WriteOwner);
        noPermFile->close();

        QVERIFY(!m_manager->importLandmarks(noPermFile, QLandmarkManager::Gpx));
        QCOMPARE(m_manager->error(), QLandmarkManager::PermissionsError); // no permissions
        noPermFile->remove();

        QVERIFY(!m_manager->importLandmarks("doesnotexist", QLandmarkManager::Gpx));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError); // file does not exist.

        QVERIFY(m_manager->importLandmarks(":data/AUS-PublicToilet-AustralianCapitalTerritory.gpx", QLandmarkManager::Gpx));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    } else if (type == "syncExcludeCategoryData") {
            QVERIFY(m_manager->importLandmarks(":data/AUS-PublicToilet-AustralianCapitalTerritory.gpx", QLandmarkManager::Gpx,
                                                QLandmarkManager::ExcludeCategoryData));
            QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    } else if (type == "syncAttachSingleCategory") {
        QVERIFY(!m_manager->importLandmarks(":data/AUS-PublicToilet-AustralianCapitalTerritory.gpx", QLandmarkManager::Gpx,
                                           QLandmarkManager::AttachSingleCategory));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError); //No category id provided

        QVERIFY(!m_manager->importLandmarks(":data/AUS-PublicToilet-AustralianCapitalTerritory.gpx", QLandmarkManager::Gpx,
                                           QLandmarkManager::AttachSingleCategory, cat3.categoryId()));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError); //Category id doesn't exist

        QVERIFY(m_manager->importLandmarks(":data/AUS-PublicToilet-AustralianCapitalTerritory.gpx", QLandmarkManager::Gpx,
                                           QLandmarkManager::AttachSingleCategory, cat2.categoryId()));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError); //valid id
    } else if (type == "async") {
        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::BadArgumentError)); //no io device set

        QFile *noPermFile = new QFile("nopermfile");
        noPermFile->open(QIODevice::WriteOnly);
        noPermFile->setPermissions(QFile::WriteOwner);
        noPermFile->close();

        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.setFileName("nopermfile");
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::PermissionsError)); //no permissions
        noPermFile->remove();

        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.setFileName("doesnotexist");
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::DoesNotExistError)); //does not exist

        importRequest.setFileName(":data/AUS-PublicToilet-AustralianCapitalTerritory.gpx");
        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::NoError,2000));
        QCOMPARE(importRequest.landmarkIds().count(),187);
    } else if (type == "asyncExcludeCategoryData") {
        importRequest.setFileName(":data/AUS-PublicToilet-AustralianCapitalTerritory.gpx");
        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.setTransferOption(QLandmarkManager::ExcludeCategoryData);
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::NoError,2000));
        QCOMPARE(importRequest.landmarkIds().count(),187);
    } else if (type == "asyncAttachSingleCategory") {
        importRequest.setFileName(":data/AUS-PublicToilet-AustralianCapitalTerritory.gpx");
        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.setTransferOption(QLandmarkManager::AttachSingleCategory);
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::DoesNotExistError)); //no category id provided

        importRequest.setFileName(":data/AUS-PublicToilet-AustralianCapitalTerritory.gpx");
        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.setTransferOption(QLandmarkManager::AttachSingleCategory);
        importRequest.setCategoryId(cat3.categoryId()); //category id doesn't exist
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::DoesNotExistError));

        importRequest.setFileName(":data/AUS-PublicToilet-AustralianCapitalTerritory.gpx");
        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.setTransferOption(QLandmarkManager::AttachSingleCategory);
        importRequest.setCategoryId(cat2.categoryId()); //valid id
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::NoError));
        QCOMPARE(importRequest.landmarkIds().count(),187);
    } else {
        qFatal("Unknown row test type");
    }

    QList<QLandmark> landmarks = m_manager->landmarks(QLandmarkFilter());
    QCOMPARE(m_manager->categories().count(),2);

    if ((type=="syncAttachSingleCategory") || (type== "asyncAttachSingleCategory")) {
        foreach(const QLandmark &landmark,landmarks) {
            QCOMPARE(landmark.categoryIds().count(),1);
            QCOMPARE(landmark.categoryIds().at(0), cat2.categoryId());
        }
        landmarks.first().setCategoryIds(QList<QLandmarkCategoryId>());
        landmarks.last().setCategoryIds(QList<QLandmarkCategoryId>());
    }

    QLandmark lmFirst;
    lmFirst.setName("Public Toilet, AUS-Wetlands Toilets");
    lmFirst.setCoordinate(QGeoCoordinate(-35.46146, 148.90686));
    lmFirst.setLandmarkId(landmarks.first().landmarkId());
    QCOMPARE(lmFirst, landmarks.first());

    QLandmark lmLast;
    lmLast.setName("Public Toilet, AUS-Kowen Forest - Playground Block");
    lmLast.setCoordinate(QGeoCoordinate(-35.32717,149.24848));
    lmLast.setLandmarkId(landmarks.last().landmarkId());

    QCOMPARE(lmLast, landmarks.last());

    QTest::qWait(10);
    QCOMPARE(spyRemove.count(), 0);
    QCOMPARE(spyChange.count(), 0);
    QCOMPARE(spyAdd.count(), 1);
    QList<QLandmarkId> ids = spyAdd.at(0).at(0).value<QList<QLandmarkId> >();
    QCOMPARE(ids.count(), 187);
    spyAdd.clear();

    if (type == "sync") {
        QVERIFY(m_manager->importLandmarks(":data/test.gpx", QLandmarkManager::Gpx));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    } else if (type == "syncExcludeCategoryData"){
        QVERIFY(m_manager->importLandmarks(":data/test.gpx", QLandmarkManager::Gpx,
                                            QLandmarkManager::ExcludeCategoryData));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    } else if (type == "syncAttachSingleCategory") {
        QVERIFY(m_manager->importLandmarks(":data/test.gpx", QLandmarkManager::Gpx,
                                           QLandmarkManager::AttachSingleCategory, cat1.categoryId()));
    } else if (type == "async") {
        importRequest.setFileName(":data/test.gpx");
        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest));
    } else if (type == "asyncExcludeCategoryData") {
        importRequest.setFileName(":data/test.gpx");
        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.setTransferOption(QLandmarkManager::ExcludeCategoryData);
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest));
    } else if (type == "asyncAttachSingleCategory") {
        importRequest.setFileName(":data/test.gpx");
        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.setTransferOption(QLandmarkManager::AttachSingleCategory);
        importRequest.setCategoryId(cat1.categoryId());
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest));
    } else {
        qFatal("Unknown test row type");
    }
    QTest::qWait(10);
    QCOMPARE(spyRemove.count(), 0);
    QCOMPARE(spyChange.count(), 0);
    QCOMPARE(spyAdd.count(), 1);
    ids = spyAdd.at(0).at(0).value<QList<QLandmarkId> >();
    QCOMPARE(ids.count(), 3);
    spyAdd.clear();

    QList<QLandmark> lms = m_manager->landmarks(ids);
    QCOMPARE(lms.count(), 3);

    QStringList lmNames;
    foreach(const QLandmark &lm, lms) {
        lmNames  << lm.name();
    }

    QVERIFY(lmNames.contains("test1"));
    QVERIFY(lmNames.contains("test2"));
    QVERIFY(lmNames.contains("test3"));

    if ((type=="syncAttachSingleCategory") || (type == "asyncAttachSingleCategory")) {
        foreach(const QLandmark &landmark,lms) {
            QCOMPARE(landmark.categoryIds().count(),1);
            QCOMPARE(landmark.categoryIds().at(0), cat1.categoryId());
        }
    }

    if (type == "async") {
        int originalLandmarksCount = m_manager->landmarks().count();
        spy.clear();
        importRequest.setFileName(":data/AUS-PublicToilet-NewSouthWales.gpx");
        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.setTransferOption(QLandmarkManager::IncludeCategoryData);
        importRequest.start();
        QTest::qWait(75);
        QCOMPARE(spy.count(),1);
        QCOMPARE(qvariant_cast<QLandmarkAbstractRequest::State>(spy.at(0).at(0)), QLandmarkAbstractRequest::ActiveState);
        importRequest.cancel();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::CancelError,2000));
        QCOMPARE(originalLandmarksCount, m_manager->landmarkIds().count());
        QCOMPARE(importRequest.landmarkIds().count(),0);

        QCOMPARE(spyRemove.count(), 0);
        QCOMPARE(spyChange.count(), 0);
        QCOMPARE(spyAdd.count(), 0);

        //check that we can use canceled request again
        importRequest.setFileName(":data/AUS-PublicToilet-AustralianCapitalTerritory.gpx");
        importRequest.setFormat(QLandmarkManager::Gpx);
        importRequest.setTransferOption(QLandmarkManager::IncludeCategoryData);
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::NoError,2000));
        QCOMPARE(originalLandmarksCount + 187, m_manager->landmarks().count());

        QCOMPARE(spyRemove.count(), 0);
        QCOMPARE(spyChange.count(), 0);
        QCOMPARE(spyAdd.count(), 1);
        QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkId> >().count(), 187);
    }

}

void tst_QLandmarkManager::importGpx_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("syncExcludeCategoryData") << "syncExcludeCategoryData";
    QTest::newRow("syncAttachSingleCategory") << "syncAttachSingleCategory";
    QTest::newRow("async") << "async";
    QTest::newRow("asyncExcludeCategoryData") << "asyncExcludeCategoryData";
    QTest::newRow("asyncAttachSingleCategory") << "asyncAttachSingleCategory";
}

void tst_QLandmarkManager::importLmx() {
    QSignalSpy spyAdd(m_manager, SIGNAL(landmarksAdded(QList<QLandmarkId>)));
    QSignalSpy spyChange(m_manager,SIGNAL(landmarksChanged(QList<QLandmarkId>)));
    QSignalSpy spyRemove(m_manager,SIGNAL(landmarksRemoved(QList<QLandmarkId>)));

    QFETCH(QString, type);
    QLandmarkImportRequest importRequest(m_manager);
    QSignalSpy spy(&importRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));

    QLandmarkCategory cat0;
    cat0.setName("cat0");
    m_manager->saveCategory(&cat0);

    QLandmarkCategory catAlpha;
    catAlpha.setName("catAlpha");

    if (type == "sync") {
        QVERIFY(m_manager->importLandmarks(":data/convert-collection-in.xml", QLandmarkManager::Lmx));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    } else if (type == "async") {
        importRequest.setFileName(":data/convert-collection-in.xml");
        importRequest.setFormat(QLandmarkManager::Lmx);
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest,QLandmarkManager::NoError));
        QCOMPARE(importRequest.landmarkIds().count(), 16);
    } else if (type == "syncExcludeCategoryData") {
        QVERIFY(m_manager->importLandmarks(":data/convert-collection-in.xml",  QLandmarkManager::Lmx,QLandmarkManager::ExcludeCategoryData));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    } else if (type == "asyncExcludeCategoryData") {
        importRequest.setFileName(":data/convert-collection-in.xml");
        importRequest.setFormat(QLandmarkManager::Lmx);
        importRequest.setTransferOption(QLandmarkManager::ExcludeCategoryData);
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest,QLandmarkManager::NoError));
        QCOMPARE(importRequest.landmarkIds().count(), 16);
    } else if (type == "syncAttachSingleCategory") {
        QVERIFY(m_manager->saveCategory(&catAlpha));

        //try with a null id
        QLandmarkCategoryId nullId;
        QVERIFY(!m_manager->importLandmarks(":data/convert-collection-in.xml", QLandmarkManager::Lmx,QLandmarkManager::AttachSingleCategory, nullId));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);

        //try with an id with the wrong manager;
        QLandmarkCategoryId wrongManagerId;
        wrongManagerId.setLocalId(cat0.categoryId().localId());
        wrongManagerId.setManagerUri("wrong.manager");
        QVERIFY(!m_manager->importLandmarks(":data/convert-collection-in.xml", QLandmarkManager::Lmx,QLandmarkManager::AttachSingleCategory, wrongManagerId));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);

        //try with the correct manager but with a non-existent localid
        QLandmarkCategoryId wrongLocalId;
        wrongLocalId.setLocalId("500");
        wrongLocalId.setManagerUri(cat0.categoryId().managerUri());
        QVERIFY(!m_manager->importLandmarks(":data/convert-collection-in.xml", QLandmarkManager::Lmx,QLandmarkManager::AttachSingleCategory, wrongLocalId));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);

        //try with a valid category id
        QVERIFY(m_manager->importLandmarks(":data/convert-collection-in.xml", QLandmarkManager::Lmx, QLandmarkManager::AttachSingleCategory,catAlpha.categoryId()));
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    } else if (type == "asyncAttachSingleCategory") {
        QVERIFY(m_manager->saveCategory(&catAlpha));

        //try with a null id
        QLandmarkCategoryId nullId;
        importRequest.setFileName(":data/convert-collection-in.xml");
        importRequest.setFormat(QLandmarkManager::Lmx);
        importRequest.setTransferOption(QLandmarkManager::AttachSingleCategory);
        importRequest.setCategoryId(nullId);
        importRequest.start();
        QVERIFY(waitForAsync(spy,&importRequest,QLandmarkManager::DoesNotExistError));

        //try with an id with the wrong manager;
        QLandmarkCategoryId wrongManagerId;
        wrongManagerId.setLocalId(cat0.categoryId().localId());
        wrongManagerId.setManagerUri("wrong.manager");
        importRequest.setFileName(":data/convert-collection-in.xml");
        importRequest.setFormat(QLandmarkManager::Lmx);
        importRequest.setTransferOption(QLandmarkManager::AttachSingleCategory);
        importRequest.setCategoryId(wrongManagerId);
        importRequest.start();
        QVERIFY(waitForAsync(spy,&importRequest,QLandmarkManager::DoesNotExistError));

        //try with the correct manager but with a non-existent localid
        QLandmarkCategoryId wrongLocalId;
        wrongLocalId.setLocalId("500");
        wrongLocalId.setManagerUri(cat0.categoryId().managerUri());
        importRequest.setFileName(":data/convert-collection-in.xml");
        importRequest.setFormat(QLandmarkManager::Lmx);
        importRequest.setTransferOption(QLandmarkManager::AttachSingleCategory);
        importRequest.setCategoryId(wrongLocalId);
        importRequest.start();
        QVERIFY(waitForAsync(spy,&importRequest,QLandmarkManager::DoesNotExistError));

        //try with a valid category id
        importRequest.setFileName(":data/convert-collection-in.xml");
        importRequest.setFormat(QLandmarkManager::Lmx);
        importRequest.setTransferOption(QLandmarkManager::AttachSingleCategory);
        importRequest.setCategoryId(catAlpha.categoryId());
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::NoError));
        QCOMPARE(importRequest.landmarkIds().count(), 16);
    } else {
        qFatal("Unknown row test type");
    }

    QTest::qWait(10);
    QCOMPARE(spyRemove.count(), 0);
    QCOMPARE(spyChange.count(), 0);
    QCOMPARE(spyAdd.count(), 1);
    QList<QLandmarkId> ids = spyAdd.at(0).at(0).value<QList<QLandmarkId> >();
    QCOMPARE(ids.count(), 16);
    spyAdd.clear();

    QList<QLandmark> landmarks = m_manager->landmarks();
    QCOMPARE(landmarks.count(), 16);

    QLandmarkNameFilter nameFilter;
    QLandmark lm;
    if ( type == "sync" || type == "async") {
        QList<QLandmarkCategory> categories = m_manager->categories();
        QCOMPARE(categories.count(), 3);

        nameFilter.setName("w16");
        landmarks = m_manager->landmarks(nameFilter);
        QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
        QCOMPARE(landmarks.count(), 1);
        lm = landmarks.at(0);
        QCOMPARE(lm.categoryIds().count(), 2);

        QSet<QString> catNames;
        foreach(const QLandmarkCategoryId &categoryId, lm.categoryIds()) {
            catNames.insert(m_manager->category(categoryId).name());
        }

        QCOMPARE(catNames.count(), 2);
        QVERIFY(catNames.contains("cat1"));
        QVERIFY(catNames.contains("cat2"));

    } else if (type == "syncExcludeCategoryData" || type == "asyncExcludeCategoryData") {
        foreach(const QLandmark &lm, landmarks) {
            QCOMPARE(lm.categoryIds().count(),0);
        }

        QCOMPARE(m_manager->categories().count(),1);
    } else if (type == "syncAttachSingleCategory" || type == "asyncAttachSingleCategory") {
        QCOMPARE(m_manager->categories().count(),2);

        foreach(const QLandmark &lm, landmarks) {
            QCOMPARE(lm.categoryIds().count(),1);
            QCOMPARE(lm.categoryIds().at(0), catAlpha.categoryId());
        }

    } else {
        qFatal("Unknown row test type");
    }

    nameFilter.setName("w0");
    landmarks = m_manager->landmarks(nameFilter);
    QCOMPARE(m_manager->error(), QLandmarkManager::NoError);
    QCOMPARE(landmarks.count(), 1);
    lm = landmarks.at(0);
    QCOMPARE(lm.name(), QString("w0"));
    QCOMPARE(lm.address().street(), QString("1 Main St"));
    QVERIFY(qFuzzyCompare(lm.coordinate().latitude(),1));
    QVERIFY(qFuzzyCompare(lm.coordinate().longitude(),2));
    if ( type == "sync" || type == "async") {
        QSet<QString> catNames;
        foreach(const QLandmarkCategoryId &categoryId, lm.categoryIds()) {
            catNames.insert(m_manager->category(categoryId).name());
        }

        QCOMPARE(catNames.count(), 2);
        QVERIFY(catNames.contains("cat0"));
        QVERIFY(catNames.contains("cat2"));
    } else if (type == "syncExcludeCategoryData"  || type == "asyncExcludeCategoryData") {
        QCOMPARE(lm.categoryIds().count(),0);
    } else if (type == "syncAttachSingleCategory" || type == "asyncAttachSingleCategory") {
        QCOMPARE(lm.categoryIds().count(),1);
        QCOMPARE(m_manager->category(lm.categoryIds().at(0)).name(), QString("catAlpha"));
    } else {
        qFatal("Unknown row test type");
    }

    if (type == "async") {
        int originalLandmarksCount = m_manager->landmarkIds().count();
        spy.clear();
        importRequest.setFileName(":data/AUS-PublicToilet-NewSouthWales.lmx");
        importRequest.setFormat(QLandmarkManager::Lmx);
        importRequest.setTransferOption(QLandmarkManager::IncludeCategoryData);
        importRequest.start();
        QTest::qWait(75);
        QCOMPARE(spy.count(),1);
        QCOMPARE(qvariant_cast<QLandmarkAbstractRequest::State>(spy.at(0).at(0)), QLandmarkAbstractRequest::ActiveState);
        importRequest.cancel();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::CancelError,2000));
        QCOMPARE(originalLandmarksCount, m_manager->landmarkIds().count());
        QCOMPARE(importRequest.landmarkIds().count(),0);

        QCOMPARE(spyRemove.count(), 0);
        QCOMPARE(spyChange.count(), 0);
        QCOMPARE(spyAdd.count(), 0);

        //check that we can use canceled request again
        importRequest.setFileName(":data/convert-collection-in.xml");
        importRequest.setFormat(QLandmarkManager::Lmx);
        importRequest.setTransferOption(QLandmarkManager::IncludeCategoryData);
        importRequest.start();
        QVERIFY(waitForAsync(spy, &importRequest, QLandmarkManager::NoError,2000));
        QCOMPARE(originalLandmarksCount + 16, m_manager->landmarks().count());

        QCOMPARE(spyRemove.count(), 0);
        QCOMPARE(spyChange.count(), 0);
        QCOMPARE(spyAdd.count(), 1);
        QCOMPARE(spyAdd.at(0).at(0).value<QList<QLandmarkId> >().count(), 16);
    }
}

void tst_QLandmarkManager::importLmx_data() {
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
    QTest::newRow("syncExcludeCategoryData") << "syncExcludeCategoryData";
    QTest::newRow("asyncExcludeCategoryData") << "asyncExcludeCategoryData";
    QTest::newRow("syncAttachSingleCategory") << "syncAttachSingleCategory";
    QTest::newRow("asyncAttachSingleCategory") << "asyncAttachSingleCategory";
}

void tst_QLandmarkManager::exportGpx() {
    QLandmarkExportRequest exportRequest(m_manager);
    QSignalSpy spy(&exportRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));

    QLandmark lm1;
    lm1.setName("lm1");
    QGeoCoordinate coord1(10,20);
    lm1.setCoordinate(coord1);
    QVERIFY(m_manager->saveLandmark(&lm1));

    QLandmark lm2;
    lm2.setName("lm2");
    QGeoCoordinate coord2(10,20);
    lm2.setCoordinate(coord2);
    QVERIFY(m_manager->saveLandmark(&lm2));

    QLandmark lm3;
    lm3.setName("lm3");
    QGeoCoordinate coord3(10,20);
    lm3.setCoordinate(coord3);
    QVERIFY(m_manager->saveLandmark(&lm3));

    //note: the gpx file handler should skip over lm4 since
    //gpx can't doesn't allow nan coordiates.
    QLandmark lm4;
    lm4.setName("lm4");
    QVERIFY(m_manager->saveLandmark(&lm4));

    QFile file(exportFile);
    if (file.exists())
        file.remove();
    QVERIFY(!file.exists());

    bool idList = false;
    QFETCH(QString, type);
    if (type == "sync") {
        QVERIFY(!m_manager->exportLandmarks(NULL,QLandmarkManager::Gpx)); //no iodevice set
        QCOMPARE(m_manager->error(), QLandmarkManager::BadArgumentError);

        QVERIFY(!m_manager->exportLandmarks(exportFile, ""));
        QCOMPARE(m_manager->error(), QLandmarkManager::BadArgumentError); //no format

        QFile *noPermFile = new QFile("nopermfile");
        noPermFile->open(QIODevice::WriteOnly);
        noPermFile->setPermissions(QFile::ReadOwner);
        noPermFile->close();

        QVERIFY(!m_manager->exportLandmarks(noPermFile, QLandmarkManager::Gpx));
        QCOMPARE(m_manager->error(), QLandmarkManager::PermissionsError); // no permissions
        noPermFile->remove();

        QVERIFY(m_manager->exportLandmarks(exportFile,QLandmarkManager::Gpx));
    } else if (type == "syncIdList") {
        QList<QLandmarkId> lmIds;

         //try an empty local id
        QLandmarkId fakeId;
        fakeId.setManagerUri(lm1.landmarkId().managerUri());
        lmIds << lm1.landmarkId() << lm2.landmarkId() << fakeId << lm3.landmarkId();
        QVERIFY(!m_manager->exportLandmarks(exportFile, QLandmarkManager::Gpx, lmIds));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);//Local id is emtpy

        //try a non-existent id
        lmIds.clear();
        fakeId.setLocalId("1000");
        lmIds << lm1.landmarkId() << lm2.landmarkId() << fakeId << lm3.landmarkId();
        QVERIFY(!m_manager->exportLandmarks(exportFile, QLandmarkManager::Lmx, lmIds));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);//id does not exist

        //try an id which refers to a landmark with doesn't have a valid coordinate for gpx
        //this should fail since we expliclty provided the id
        lmIds.clear();
        lmIds << lm1.landmarkId() << lm2.landmarkId() << lm4.landmarkId();
        QVERIFY(!m_manager->exportLandmarks(exportFile, QLandmarkManager::Gpx, lmIds));
        QCOMPARE(m_manager->error(), QLandmarkManager::BadArgumentError);//id does not exist

        lmIds.clear();
        lmIds << lm2.landmarkId() << lm3.landmarkId();
        QVERIFY(m_manager->exportLandmarks(exportFile, QLandmarkManager::Gpx, lmIds));
        idList = true;
    } else if (type == "syncExcludeCategoryData") {
        QVERIFY(m_manager->exportLandmarks(exportFile,QLandmarkManager::Gpx, QList<QLandmarkId>(), QLandmarkManager::ExcludeCategoryData));
    } else if (type == "syncAttachSingleCategory") {
        QVERIFY(m_manager->exportLandmarks(exportFile,QLandmarkManager::Gpx, QList<QLandmarkId>(), QLandmarkManager::AttachSingleCategory));
    } else if (type == "async"){
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest,QLandmarkManager::BadArgumentError));//no iodevice set
        exportRequest.setFileName(exportFile);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::BadArgumentError)); //no format

        QFile *noPermFile = new QFile("nopermfile");
        noPermFile->open(QIODevice::WriteOnly);
        noPermFile->setPermissions(QFile::ReadOwner);
        noPermFile->close();

        exportRequest.setDevice(noPermFile);
        exportRequest.setFormat(QLandmarkManager::Gpx);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::PermissionsError));
        noPermFile->remove();

        exportRequest.setFileName(exportFile);
        exportRequest.setFormat(QLandmarkManager::Gpx);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::NoError));
        spy.clear();
    } else if (type == "asyncIdList") {
        QList<QLandmarkId> lmIds;

         //try an empty local id
        QLandmarkId fakeId;
        fakeId.setManagerUri(lm1.landmarkId().managerUri());
        lmIds << lm1.landmarkId() << lm2.landmarkId() << fakeId << lm3.landmarkId();
        exportRequest.setFileName(exportFile);
        exportRequest.setFormat(QLandmarkManager::Gpx);
        exportRequest.setLandmarkIds(lmIds);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::DoesNotExistError)); //local id is empty

        //try a non-existent id
        lmIds.clear();
        fakeId.setLocalId("1000");
        lmIds << lm1.landmarkId() << lm2.landmarkId() << fakeId << lm3.landmarkId();
        exportRequest.setFileName(exportFile);
        exportRequest.setFormat(QLandmarkManager::Gpx);
        exportRequest.setLandmarkIds(lmIds);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::DoesNotExistError)); //local id is empty

        //try an id which refers to a landmark with doesn't have a valid coordinate for gpx
        //this should fail since we expliclty provided the id
        lmIds.clear();
        lmIds << lm1.landmarkId() << lm2.landmarkId() << lm4.landmarkId();
        exportRequest.setFileName(exportFile);
        exportRequest.setFormat(QLandmarkManager::Gpx);
        exportRequest.setLandmarkIds(lmIds);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::BadArgumentError));

        lmIds.clear();
        lmIds << lm2.landmarkId() << lm3.landmarkId();
        exportRequest.setFileName(exportFile);
        exportRequest.setFormat(QLandmarkManager::Gpx);
        exportRequest.setLandmarkIds(lmIds);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest));
        idList = true;
    } else if (type == "asyncExcludeCategoryData") {
        exportRequest.setFileName(exportFile);
        exportRequest.setFormat(QLandmarkManager::Gpx);
        exportRequest.setTransferOption(QLandmarkManager::ExcludeCategoryData);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest));
    } else if(type == "asyncAttachSingleCategory"){
        exportRequest.setFormat(QLandmarkManager::Gpx);
        exportRequest.setTransferOption(QLandmarkManager::AttachSingleCategory);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest));
    } else {
        qFatal("Unrecognised test row");
    }

    QVERIFY(QFile::exists(exportFile));

    QVERIFY(m_manager->importLandmarks(exportFile, QLandmarkManager::Gpx));

    QList<QLandmark> lms = m_manager->landmarks();
    QLandmarkNameFilter nameFilter;

    if (!idList) {
        QCOMPARE(lms.count(), 7);
        nameFilter.setName("lm1");
        QCOMPARE(m_manager->landmarks(nameFilter).count(),2);
        nameFilter.setName("lm2");
        QCOMPARE(m_manager->landmarks(nameFilter).count(),2);
        nameFilter.setName("lm3");
        QCOMPARE(m_manager->landmarks(nameFilter).count(),2);
        nameFilter.setName("lm4");
        QCOMPARE(m_manager->landmarks(nameFilter).count(),1);
    } else if (idList) {
        QCOMPARE(lms.count(), 6);
        nameFilter.setName("lm1");
        QCOMPARE(m_manager->landmarks(nameFilter).count(),1);
        nameFilter.setName("lm2");
        QCOMPARE(m_manager->landmarks(nameFilter).count(),2);
        nameFilter.setName("lm3");
        QCOMPARE(m_manager->landmarks(nameFilter).count(),2);
        nameFilter.setName("lm4");
        QCOMPARE(m_manager->landmarks(nameFilter).count(),1);
    }

    if (type == "async") {
        QFile::remove(exportFile);
        QLandmark lm;
        lms.clear();
        for (int i=0; i < 600; ++i) {
            lm.setName(QString("LM%1").arg(0));
            lms.append(lm);
        }

        QVERIFY(m_manager->saveLandmarks(&lms));
        exportRequest.setFormat(QLandmarkManager::Gpx);
        exportRequest.setTransferOption(QLandmarkManager::IncludeCategoryData);
        exportRequest.setFileName(exportFile);
        exportRequest.setLandmarkIds(QList<QLandmarkId>());
        exportRequest.start();
        QTest::qWait(50);
        exportRequest.cancel();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::CancelError,2000));
    }

}

void tst_QLandmarkManager::exportGpx_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("syncIdList") << "syncIdList";
    QTest::newRow("syncExcludeCategoryData") << "syncExcludeCategoryData";
    QTest::newRow("async") << "async";
    QTest::newRow("asyncIdList") << "asyncIdList";
    QTest::newRow("asyncExcludeCategoryData") << "asyncExcludeCategoryData";
}

void tst_QLandmarkManager::exportLmx() {
    //Note: lmx does not support iconUrl, countryCode, streetNumber
    QString lm1Name("lm1 name");
    QString lm1Description("lm1 Description");
    qreal lm1Radius(5);
    QString lm1PhoneNumber("lm1 phoneNumber");
    QUrl lm1Url("lm1 URL");
    QGeoCoordinate lm1Coordinate(1,2,3);
    QGeoAddress lm1Address;
    lm1Address.setCountry("lm1 country");
    lm1Address.setState("lm1 state");
    lm1Address.setCounty("lm1 county");
    lm1Address.setCity("lm1 city");
    lm1Address.setDistrict("lm1 district");
    lm1Address.setStreet("lm1 street");
    lm1Address.setPostCode("lm1 postCode");
    QLandmark lm1;
    lm1.setName(lm1Name);
    lm1.setDescription(lm1Description);
    lm1.setRadius(lm1Radius);
    lm1.setPhoneNumber(lm1PhoneNumber);
    lm1.setUrl(lm1Url);
    lm1.setCoordinate(lm1Coordinate);
    lm1.setAddress(lm1Address);

    QString lm2Name("lm2 name");
    QString lm2Description("lm2 Description");
    qreal lm2Radius(6);
    QString lm2PhoneNumber("lm2 phoneNumber");
    QUrl lm2Url("lm2 URL");
    QGeoCoordinate lm2Coordinate(4,5,6);
    QGeoAddress lm2Address;
    lm2Address.setCountry("lm2 country");
    lm2Address.setState("lm2 state");
    lm2Address.setCounty("lm2 county");
    lm2Address.setCity("lm2 city");
    lm2Address.setDistrict("lm2 district");
    lm2Address.setStreet("lm2 street");
    lm2Address.setPostCode("lm2 postCode");
    QLandmark lm2;
    lm2.setName(lm2Name);
    lm2.setDescription(lm2Description);
    lm2.setRadius(lm2Radius);
    lm2.setPhoneNumber(lm2PhoneNumber);
    lm2.setUrl(lm2Url);
    lm2.setCoordinate(lm2Coordinate);
    lm2.setAddress(lm2Address);

    QString lm3Name("lm3 name");
    QString lm3Description("lm3 Description");
    qreal lm3Radius(6);
    QString lm3PhoneNumber("lm3 phoneNumber");
    QUrl lm3Url("lm3 URL");
    QGeoCoordinate lm3Coordinate(4,5,6);
    QGeoAddress lm3Address;
    lm3Address.setCountry("lm3 country");
    lm3Address.setState("lm3 state");
    lm3Address.setCounty("lm3 county");
    lm3Address.setCity("lm3 city");
    lm3Address.setDistrict("lm3 district");
    lm3Address.setStreet("lm3 street");
    lm3Address.setPostCode("lm3 postCode");
    QLandmark lm3;
    lm3.setName(lm3Name);
    lm3.setDescription(lm3Description);
    lm3.setRadius(lm3Radius);
    lm3.setPhoneNumber(lm3PhoneNumber);
    lm3.setUrl(lm3Url);
    lm3.setCoordinate(lm3Coordinate);
    lm3.setAddress(lm3Address);

    QString cat1Name("cat1 name");
    QString cat1IconUrl("cat1 iconUrl");
    QLandmarkCategory cat1;
    cat1.setName(cat1Name);
    cat1.setIconUrl(cat1IconUrl);

    QString cat2Name("cat2 name");
    QString cat2IconUrl("cat2 iconUrl");
    QLandmarkCategory cat2;
    cat2.setName(cat2Name);
    cat2.setIconUrl(cat2IconUrl);

    QString cat3Name("cat3 name");
    QString cat3IconUrl("cat3 iconUrl");
    QLandmarkCategory cat3;
    cat3.setName(cat3Name);
    cat3.setIconUrl(cat3IconUrl);

    QVERIFY(m_manager->saveCategory(&cat1));
    QVERIFY(m_manager->saveCategory(&cat2));
    QVERIFY(m_manager->saveCategory(&cat3));

    lm1.addCategoryId(cat1.categoryId());
    lm1.addCategoryId(cat2.categoryId());
    lm1.addCategoryId(cat3.categoryId());
    lm2.addCategoryId(cat2.categoryId());

    QVERIFY(m_manager->saveLandmark(&lm1));
    QVERIFY(m_manager->saveLandmark(&lm2));
    QVERIFY(m_manager->saveLandmark(&lm3));

    QLandmarkExportRequest exportRequest(m_manager);
    QSignalSpy spy(&exportRequest, SIGNAL(stateChanged(QLandmarkAbstractRequest::State)));
    QFETCH(QString, type);

    bool includeCategoryData = true;
    bool idList = false;
    if (type== "sync") {
        QVERIFY(!m_manager->exportLandmarks(NULL,QLandmarkManager::Lmx)); //no iodevice set
        QCOMPARE(m_manager->error(), QLandmarkManager::BadArgumentError);

        QVERIFY(!m_manager->exportLandmarks(exportFile, ""));
        QCOMPARE(m_manager->error(), QLandmarkManager::BadArgumentError); //no format

        QFile *noPermFile = new QFile("nopermfile");
        noPermFile->open(QIODevice::WriteOnly);
        noPermFile->setPermissions(QFile::ReadOwner);
        noPermFile->close();

        QVERIFY(!m_manager->exportLandmarks(noPermFile, QLandmarkManager::Lmx));
        QCOMPARE(m_manager->error(), QLandmarkManager::PermissionsError); // no permissions
        noPermFile->remove();

        QVERIFY(m_manager->exportLandmarks(exportFile, QLandmarkManager::Lmx));
    } else if (type == "syncIdList") {
        QList<QLandmarkId> lmIds;

         //try an empty local id
        QLandmarkId fakeId;
        fakeId.setManagerUri(lm1.landmarkId().managerUri());
        lmIds << lm1.landmarkId() << lm2.landmarkId() << fakeId << lm3.landmarkId();
        QVERIFY(!m_manager->exportLandmarks(exportFile, QLandmarkManager::Lmx, lmIds));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);//Local id is emtpy

        //try a non-existent id
        lmIds.clear();
        fakeId.setLocalId("1000");
        lmIds << lm1.landmarkId() << lm2.landmarkId() << fakeId << lm3.landmarkId();
        QVERIFY(!m_manager->exportLandmarks(exportFile, QLandmarkManager::Lmx, lmIds));
        QCOMPARE(m_manager->error(), QLandmarkManager::DoesNotExistError);//id does not exist

        lmIds.clear();
        lmIds << lm1.landmarkId() << lm3.landmarkId();
        QVERIFY(m_manager->exportLandmarks(exportFile, QLandmarkManager::Lmx, lmIds));
        idList = true;
    } else if (type == "syncExcludeCategoryData") {
        QVERIFY(m_manager->exportLandmarks(exportFile, QLandmarkManager::Lmx, QList<QLandmarkId>(), QLandmarkManager::ExcludeCategoryData));
        includeCategoryData = false;
    } else if (type == "syncAttachSingleCategory") {
        QVERIFY(m_manager->exportLandmarks(exportFile, QLandmarkManager::Lmx,QList<QLandmarkId>(),QLandmarkManager::AttachSingleCategory));
    } else if (type == "async") {
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest,QLandmarkManager::BadArgumentError));//no iodevice set
        exportRequest.setFileName(exportFile);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::BadArgumentError)); //no format

        QFile *noPermFile = new QFile("nopermfile");
        noPermFile->open(QIODevice::WriteOnly);
        noPermFile->setPermissions(QFile::ReadOwner);
        noPermFile->close();

        exportRequest.setDevice(noPermFile);
        exportRequest.setFormat(QLandmarkManager::Lmx);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::PermissionsError));
        noPermFile->remove();

        exportRequest.setFormat(QLandmarkManager::Lmx);
        exportRequest.setFileName(exportFile);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest));
    } else if (type == "asyncIdList") {
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest,QLandmarkManager::BadArgumentError));//no iodevice set

        exportRequest.setFileName(exportFile);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::BadArgumentError)); //no format

        QList<QLandmarkId> lmIds;

        //try an empty local id
        QLandmarkId fakeId;
        fakeId.setManagerUri(lm1.landmarkId().managerUri());
        lmIds << lm1.landmarkId() << lm2.landmarkId() << fakeId << lm3.landmarkId();
        exportRequest.setFormat(QLandmarkManager::Lmx);
        exportRequest.setLandmarkIds(lmIds);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::DoesNotExistError)); //local id is empty

        //try a non existent id
        lmIds.clear();
        fakeId.setLocalId("1000");
        lmIds << lm1.landmarkId() << lm2.landmarkId() << fakeId << lm3.landmarkId();
        exportRequest.setFormat(QLandmarkManager::Lmx);
        exportRequest.setLandmarkIds(lmIds);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::DoesNotExistError)); //local id is empty

        lmIds.clear();
        lmIds << lm1.landmarkId() << lm3.landmarkId();
        exportRequest.setFormat(QLandmarkManager::Lmx);
        exportRequest.setLandmarkIds(lmIds);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest));
        idList = true;
    } else if (type == "asyncExcludeCategoryData") {
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest,QLandmarkManager::BadArgumentError));//no iodevice set
        exportRequest.setFileName(exportFile);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::BadArgumentError)); //no format
        exportRequest.setFormat(QLandmarkManager::Lmx);
        exportRequest.setTransferOption(QLandmarkManager::ExcludeCategoryData);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest));
        includeCategoryData = false;
    } else if (type == "asyncAttachSingleCategory") {
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest,QLandmarkManager::BadArgumentError));//no iodevice set
        exportRequest.setFileName(exportFile);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::BadArgumentError)); //no format
        exportRequest.setFormat(QLandmarkManager::Lmx);
        exportRequest.setTransferOption(QLandmarkManager::AttachSingleCategory);
        exportRequest.start();
        QVERIFY(waitForAsync(spy, &exportRequest));
    } else {
        qFatal("Unrecognised test row");
    }

    QVERIFY(m_manager->removeLandmarks(m_manager->landmarkIds()));
    QList<QLandmarkCategoryId> catIds = m_manager->categoryIds();
    foreach(const QLandmarkCategoryId &catId, catIds) {
        m_manager->removeCategory(catId);
    }

    QVERIFY(m_manager->landmarks().isEmpty());
    QVERIFY(m_manager->categories().isEmpty());

    QVERIFY(m_manager->importLandmarks(exportFile, QLandmarkManager::Lmx));


    QList<QLandmark> lms = m_manager->landmarks();

    QLandmark lm1New;
    QLandmark lm2New;
    QLandmark lm3New;

    if (!idList) {
        QCOMPARE(lms.count(), 3);
        lm1New = lms.at(0);
        lm2New = lms.at(1);
        lm3New = lms.at(2);
    } else {
        QCOMPARE(lms.count(), 2);
        lm1New = lms.at(0);
        lm3New = lms.at(1);
    }

    QCOMPARE(lm1New.name(), lm1Name);
    QCOMPARE(lm1New.description(), lm1Description);
    QCOMPARE(lm1New.iconUrl(), QUrl());
    QCOMPARE(lm1New.radius(), lm1Radius);
    QCOMPARE(lm1New.phoneNumber(), lm1PhoneNumber);
    QCOMPARE(lm1New.url(),lm1Url);
    QCOMPARE(lm1New.coordinate(),lm1Coordinate);
    QCOMPARE(lm1New.address().country(), lm1Address.country());
    QCOMPARE(lm1New.address().state(),lm1Address.state());
    QCOMPARE(lm1New.address().county(), lm1Address.county());
    QCOMPARE(lm1New.address().city(), lm1Address.city());
    QCOMPARE(lm1New.address().district(), lm1Address.district());
    QCOMPARE(lm1New.address().street(), lm1Address.street());
    QCOMPARE(lm1New.address().postCode(), lm1Address.postCode());

    if (includeCategoryData) {
        QCOMPARE(lm1.categoryIds().count(),3);
        QList<QLandmarkCategory> cats = m_manager->categories(lm1.categoryIds());
        QCOMPARE(cats.count(),3);
        QLandmarkCategory cat1New = cats.at(0);
        QLandmarkCategory cat2New = cats.at(1);
        QLandmarkCategory cat3New = cats.at(2);
        QCOMPARE(cat1New.name(), cat1.name());
        QCOMPARE(cat2New.name(), cat2.name());
        QCOMPARE(cat3New.name(), cat3.name());
    } else {
        lm1.setCategoryIds(QList<QLandmarkCategoryId>());
        lm2.setCategoryIds(QList<QLandmarkCategoryId>());
        lm3.setCategoryIds(QList<QLandmarkCategoryId>());
        QList<QLandmarkCategory> cats = m_manager->categories(lm1.categoryIds());
        QCOMPARE(cats.count(),0);
    }

    if (!idList) {
        QCOMPARE(lm1New, lm1);
        QCOMPARE(lm2New, lm2);
        QCOMPARE(lm3New, lm3);
    } else {
        QCOMPARE(lm1New, lm1);
        lm3.setLandmarkId(lm2.landmarkId());  //3 will get assigned the same id as 2
        QCOMPARE(lm3New, lm3);
    }

    if (type == "async") {
        QFile::remove(exportFile);
        QLandmark lm;
        lms.clear();
        for (int i=0; i < 600; ++i) {
            lm.setName(QString("LM%1").arg(0));
            lms.append(lm);
        }

        QVERIFY(m_manager->saveLandmarks(&lms));
        exportRequest.setFormat(QLandmarkManager::Lmx);
        exportRequest.setTransferOption(QLandmarkManager::IncludeCategoryData);
        exportRequest.setFileName(exportFile);
        exportRequest.setLandmarkIds(QList<QLandmarkId>());
        exportRequest.start();
        QTest::qWait(50);
        exportRequest.cancel();
        QVERIFY(waitForAsync(spy, &exportRequest, QLandmarkManager::CancelError,2500));
    }
    QFile::remove(exportFile);
}

void tst_QLandmarkManager::exportLmx_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("syncIdList") << "syncIdList";
    QTest::newRow("syncExcludeCategoryData") << "syncExcludeCategoryData";
    QTest::newRow("syncAttachSingleCategory") << "syncAttachSingleCategory";
    QTest::newRow("async") << "async";
    QTest::newRow("asyncIdList") << "asyncIdList";
    QTest::newRow("asyncExcludeCategoryData") << "asyncExcludeCategoryData";
    QTest::newRow("asyncAttachSingleCategory") << "asyncAttachSingleCategory";

    //TODO: tests for id list excluding category data
}

void tst_QLandmarkManager::importFile()
{
    QFETCH(QString, type);
    //try a gpx file
    doImport(type, ":data/test.gpx");
    QCOMPARE(m_manager->landmarks().count(), 3);
    QVERIFY(m_manager->removeLandmarks(m_manager->landmarkIds()));
    QCOMPARE(m_manager->landmarks().count(), 0);

    //try an lmx file
    doImport(type,":data/convert-collection-in.xml");
    QCOMPARE(m_manager->landmarks().count(), 16);
    QCOMPARE(m_manager->categories().count(), 3);
    QVERIFY(m_manager->removeLandmarks(m_manager->landmarkIds()));
    QCOMPARE(m_manager->landmarks().count(), 0);
    QList<QLandmarkCategoryId> catIds = m_manager->categoryIds();
    for (int i=0; i < catIds.count() ; ++i) {
        QVERIFY(m_manager->removeCategory(catIds.at(i)));
    }
    QCOMPARE(m_manager->categories().count(), 0);

    //try an invalid format
    doImport(type, ":data/file.omg", QLandmarkManager::NotSupportedError);

    //try an invalid file
    doImport(type, ":data/garbage.xml", QLandmarkManager::ParsingError);
}

void tst_QLandmarkManager::importFile_data()
{
    QTest::addColumn<QString>("type");

    QTest::newRow("sync") << "sync";
    QTest::newRow("async") << "async";
}

void tst_QLandmarkManager::supportedFormats() {
        QStringList formats = m_manager->supportedFormats(QLandmarkManager::ExportOperation);
        QCOMPARE(formats.count(), 2);
        QVERIFY(formats.at(0) == QLandmarkManager::Gpx);
        QVERIFY(formats.at(1) == QLandmarkManager::Lmx);

        formats = m_manager->supportedFormats(QLandmarkManager::ImportOperation);
        QCOMPARE(formats.count(), 2);
        QVERIFY(formats.at(0) == QLandmarkManager::Gpx);
        QVERIFY(formats.at(1) == QLandmarkManager::Lmx);
}

void tst_QLandmarkManager::filterSupportLevel() {
    QLandmarkFilter filter;
    QCOMPARE(m_manager->filterSupportLevel(filter), QLandmarkManager::NativeSupport);
    //TODO: Invalid filter?

    //name filter
    QLandmarkNameFilter nameFilter;
    QCOMPARE(m_manager->filterSupportLevel(nameFilter), QLandmarkManager::NativeSupport);
    nameFilter.setMatchFlags(QLandmarkFilter::MatchStartsWith);
    QCOMPARE(m_manager->filterSupportLevel(nameFilter), QLandmarkManager::NativeSupport);
    nameFilter.setMatchFlags(QLandmarkFilter::MatchContains);
    QCOMPARE(m_manager->filterSupportLevel(nameFilter), QLandmarkManager::NativeSupport);
    nameFilter.setMatchFlags(QLandmarkFilter::MatchEndsWith);
    QCOMPARE(m_manager->filterSupportLevel(nameFilter), QLandmarkManager::NativeSupport);
    nameFilter.setMatchFlags(QLandmarkFilter::MatchFixedString);
    QCOMPARE(m_manager->filterSupportLevel(nameFilter), QLandmarkManager::NativeSupport);
    nameFilter.setMatchFlags(QLandmarkFilter::MatchExactly);
    QCOMPARE(m_manager->filterSupportLevel(nameFilter), QLandmarkManager::NativeSupport);
    nameFilter.setMatchFlags(QLandmarkFilter::MatchCaseSensitive);
    QCOMPARE(m_manager->filterSupportLevel(nameFilter), QLandmarkManager::NoSupport);
    nameFilter.setMatchFlags(QLandmarkFilter::MatchCaseSensitive | QLandmarkFilter::MatchStartsWith);
    QCOMPARE(m_manager->filterSupportLevel(nameFilter), QLandmarkManager::NoSupport);

    //proximity filter
    QLandmarkProximityFilter proximityFilter;
    QCOMPARE(m_manager->filterSupportLevel(proximityFilter), QLandmarkManager::NativeSupport);

    //box filter
    QLandmarkBoxFilter boxFilter;
    QCOMPARE(m_manager->filterSupportLevel(boxFilter), QLandmarkManager::NativeSupport);

    //AND filter
    QLandmarkIntersectionFilter andFilter;
    QCOMPARE(m_manager->filterSupportLevel(andFilter), QLandmarkManager::NativeSupport);
    andFilter.append(boxFilter);
    andFilter.append(proximityFilter);
    QCOMPARE(m_manager->filterSupportLevel(andFilter), QLandmarkManager::NativeSupport);
    nameFilter.setMatchFlags(QLandmarkFilter::MatchCaseSensitive);
    andFilter.append(nameFilter);
    andFilter.append(filter);
    QCOMPARE(m_manager->filterSupportLevel(andFilter), QLandmarkManager::NoSupport);
    andFilter.clear();

    QLandmarkIntersectionFilter andFilter2;//try multi level intersection
    andFilter2.append(filter);             //that has native support
    andFilter.append(boxFilter);
    andFilter.append(andFilter2);
    QCOMPARE(m_manager->filterSupportLevel(andFilter), QLandmarkManager::NativeSupport);
    andFilter2.append(nameFilter);  //try a multi level intersection with no
    andFilter.clear();              //support
    andFilter.append(boxFilter);
    andFilter.append(andFilter2);
    andFilter.append(proximityFilter);
    QCOMPARE(m_manager->filterSupportLevel(andFilter), QLandmarkManager::NoSupport);


    //union filter
    QLandmarkUnionFilter orFilter;
    QCOMPARE(m_manager->filterSupportLevel(orFilter), QLandmarkManager::NativeSupport);
    orFilter.append(boxFilter);
    orFilter.append(proximityFilter);
    QCOMPARE(m_manager->filterSupportLevel(orFilter), QLandmarkManager::NativeSupport);
    nameFilter.setMatchFlags(QLandmarkFilter::MatchCaseSensitive);
    orFilter.append(nameFilter);
    orFilter.append(filter);
    QCOMPARE(m_manager->filterSupportLevel(orFilter), QLandmarkManager::NoSupport);
    orFilter.clear();

    QLandmarkUnionFilter orFilter2;//try multi level Union
    orFilter2.append(filter);             //that has native support
    orFilter.append(boxFilter);
    orFilter.append(orFilter2);
    QCOMPARE(m_manager->filterSupportLevel(orFilter), QLandmarkManager::NativeSupport);
    orFilter2.append(nameFilter);  //try a multi level Union with no
    orFilter.clear();              //support
    orFilter.append(boxFilter);
    orFilter.append(orFilter2);
    orFilter.append(proximityFilter);
    QCOMPARE(m_manager->filterSupportLevel(orFilter), QLandmarkManager::NoSupport);

    //attribute filter
    //manager attributes that exist
    QLandmarkAttributeFilter attributeFilter;
    attributeFilter.setAttribute("name", "jack");
    attributeFilter.setAttribute("description", "colonel");
    QCOMPARE(m_manager->filterSupportLevel(attributeFilter), QLandmarkManager::NativeSupport);

    //try a manager attribute that doesn't exist
    attributeFilter.setAttribute("weapon", "staff");
    QCOMPARE(m_manager->filterSupportLevel(attributeFilter), QLandmarkManager::NoSupport);

    //try an attribute with case sensitive matching(not supported
    attributeFilter.clearAttributes();
    attributeFilter.setAttribute("description", "desc", QLandmarkFilter::MatchCaseSensitive);
    attributeFilter.setAttribute("street", "abydos");
    QCOMPARE(m_manager->filterSupportLevel(attributeFilter), QLandmarkManager::NoSupport);
    attributeFilter.setAttribute("description", "desc",
                    QLandmarkFilter::MatchCaseSensitive | QLandmarkFilter::MatchStartsWith);
    attributeFilter.setAttribute("street", "abydos");
    QCOMPARE(m_manager->filterSupportLevel(attributeFilter), QLandmarkManager::NoSupport);

    //try see if other match flags will give native support
    attributeFilter.setAttribute("description", "desc");
    attributeFilter.setAttribute("street", "abydos", QLandmarkFilter::MatchStartsWith);
    QCOMPARE(m_manager->filterSupportLevel(attributeFilter), QLandmarkManager::NativeSupport);

    //try a landmark id filter
    QLandmarkIdFilter idFilter;
    QCOMPARE(m_manager->filterSupportLevel(idFilter), QLandmarkManager::NativeSupport);
}

void tst_QLandmarkManager::sortOrderSupportLevel() {
    //default sort order
    QLandmarkSortOrder defaultSort;
    QList<QLandmarkSortOrder> sortOrders;
    sortOrders << defaultSort;
    QCOMPARE(m_manager->sortOrderSupportLevel(sortOrders), QLandmarkManager::NativeSupport);

    //name sort order
    QLandmarkNameSort nameSort;
    sortOrders.clear();
    sortOrders << nameSort;
    QCOMPARE(m_manager->sortOrderSupportLevel(sortOrders), QLandmarkManager::NativeSupport);

    //try a list
    sortOrders.clear();
    sortOrders << defaultSort << nameSort << defaultSort;
    QCOMPARE(m_manager->sortOrderSupportLevel(sortOrders), QLandmarkManager::NativeSupport);
}

void tst_QLandmarkManager::isFeatureSupported()
{
    QVERIFY(m_manager->isFeatureSupported(QLandmarkManager::NotificationsFeature));
    QVERIFY(m_manager->isFeatureSupported(QLandmarkManager::ImportExportFeature));
    //TODO: remove custom attributes, QVERIFY(m_manager->isFeatureSupported(QLandmarkManager::CustomAttributesFeature));
}

void tst_QLandmarkManager::categoryLimitOffset() {
    for (int i = 0; i < 50; ++i) {
        QLandmarkCategory cat;
        cat.setName(QString("CAT%1").arg(i));
        QVERIFY(m_manager->saveCategory(&cat));
    }

    QList<QLandmarkCategoryId> ids = m_manager->categoryIds();
    QCOMPARE(ids.size(), 50);

    //default

    ids = m_manager->categoryIds(100, 0);

    QCOMPARE(ids.size(), 50);

    QLandmarkNameSort nameSort(Qt::DescendingOrder);

    ids = m_manager->categoryIds(25,0, nameSort);

    QCOMPARE(ids.size(), 25);
    QCOMPARE(m_manager->category(ids.at(0)).name(), QString("CAT9"));
    QCOMPARE(m_manager->category(ids.at(24)).name(), QString("CAT31"));

    QList<QLandmarkCategory> cats = m_manager->categories(25, 0, nameSort);
    QCOMPARE(cats.size(), 25);
    QCOMPARE(cats.at(0).name(), QString("CAT9"));
    QCOMPARE(cats.at(24).name(), QString("CAT31"));

    //try with an limit and offset
    cats = m_manager->categories(10, 10, nameSort);
    ids = m_manager->categoryIds(10, 10, nameSort);
    QVERIFY(checkIds(cats, ids));

    QCOMPARE(cats.size(), 10);
    QCOMPARE(cats.at(0).name(), QString("CAT44"));
    QCOMPARE(cats.at(9).name(), QString("CAT36"));

    //try with an offset and no max items
    cats = m_manager->categories(-1,10,QLandmarkNameSort(Qt::AscendingOrder));
    ids = m_manager->categoryIds(-1, 10, QLandmarkNameSort(Qt::AscendingOrder));
    QVERIFY(checkIds(cats, ids));

    QCOMPARE(cats.size(), 40);
    QCOMPARE(cats.at(0).name(), QString("CAT18"));
    QCOMPARE(cats.at(39).name(), QString("CAT9"));

    //try with an offset of -1
    cats = m_manager->categories(-1,-1, QLandmarkNameSort(Qt::AscendingOrder));
    ids = m_manager->categoryIds(-1,-1, QLandmarkNameSort(Qt::AscendingOrder));
    QVERIFY(checkIds(cats, ids));

    //try with an offset which greater than the number of items
    cats = m_manager->categories( 100, 500, QLandmarkNameSort(Qt::AscendingOrder));
    QCOMPARE(cats.count(), 0);
}

void tst_QLandmarkManager::notificationCheck()
{
    //check that we don't receive notifications prior of events(ie landmark added/modified)
    //made to the creation of a landmark  manager
    QSignalSpy spyCatAdd(m_manager, SIGNAL(categoriesAdded(QList<QLandmarkCategoryId>)));
    QSignalSpy spyLmAdd(m_manager, SIGNAL(landmarksAdded(QList<QLandmarkId>)));

    QMap<QString,QString> parameters;
    parameters.insert("filename", "test.db");

    QLandmark lm1;
    lm1.setName("LM1");
    m_manager->saveLandmark(&lm1);

    QTest::qWait(10);
    QCOMPARE(spyCatAdd.count(),0);
    QCOMPARE(spyLmAdd.count(),1);
    delete m_manager;
#ifdef Q_OS_SYMBIAN
    m_manager = new QLandmarkManager();
#else
    m_manager = new QLandmarkManager("com.nokia.qt.landmarks.engines.sqlite", parameters);
#endif
    connectNotifications();

    QSignalSpy spyCatAdd2(m_manager, SIGNAL(categoriesAdded(QList<QLandmarkCategoryId>)));
    QSignalSpy spyLmAdd2(m_manager, SIGNAL(landmarksAdded(QList<QLandmarkId>)));

     QLandmarkCategory cat1;
     cat1.setName("CAT1");
     QVERIFY(m_manager->saveCategory(&cat1));

    QTest::qWait(10);
    QCOMPARE(spyCatAdd2.count(),1);
    QCOMPARE(spyLmAdd2.count(),0);
}

void tst_QLandmarkManager::testConvenienceFunctions()
{
    QGeoBoundingCircle circle;
    circle.setCenter(QGeoCoordinate(10,10));
    circle.setRadius(5000);
    QLandmarkProximityFilter proximityFilter(circle);

    QVERIFY(qFuzzyCompare(proximityFilter.radius(),5000));
    QVERIFY(qFuzzyCompare(proximityFilter.boundingCircle().radius(), 5000));
    QCOMPARE(proximityFilter.center(),QGeoCoordinate(10,10));
    QCOMPARE(proximityFilter.boundingCircle().center(),QGeoCoordinate(10,10));

    //try setting radius
    proximityFilter.setRadius(1000);
    QVERIFY(qFuzzyCompare(proximityFilter.radius(),1000));
    QVERIFY(qFuzzyCompare(proximityFilter.boundingCircle().radius(),1000));

    //try setting the coordinate
    proximityFilter.setCenter(QGeoCoordinate(20,20));
    QCOMPARE(proximityFilter.center(), QGeoCoordinate(20,20));
    QCOMPARE(proximityFilter.boundingCircle().center(), QGeoCoordinate(20,20));

    //try setting the circle
    circle.setCenter(QGeoCoordinate(30,30));
    circle.setRadius(2000);
    proximityFilter.setBoundingCircle(circle);
    QVERIFY(qFuzzyCompare(proximityFilter.radius(),2000));
    QVERIFY(qFuzzyCompare(proximityFilter.boundingCircle().radius(),2000));
    QCOMPARE(proximityFilter.center(),QGeoCoordinate(30,30));
    QCOMPARE(proximityFilter.boundingCircle().center(), QGeoCoordinate(30,30));

    QGeoBoundingBox box(QGeoCoordinate(50,50), QGeoCoordinate(20,100));
    QLandmarkBoxFilter boxFilter(box);
    QCOMPARE(boxFilter.topLeft(), QGeoCoordinate(50,50));
    QCOMPARE(boxFilter.bottomRight(), QGeoCoordinate(20,100));

    //set the top left
    boxFilter.setTopLeft(QGeoCoordinate(49,49));
    QCOMPARE(boxFilter.topLeft(), QGeoCoordinate(49,49));
    QCOMPARE(boxFilter.boundingBox().topLeft(), QGeoCoordinate(49,49));

    //set bottom Right
    boxFilter.setBottomRight(QGeoCoordinate(19,99));
    QCOMPARE(boxFilter.bottomRight(), QGeoCoordinate(19,99));
    QCOMPARE(boxFilter.boundingBox().bottomRight(), QGeoCoordinate(19,99));

    //set the bounding box
    box.setTopLeft(QGeoCoordinate(30,30));
    box.setBottomRight(QGeoCoordinate(5,90));
    boxFilter.setBoundingBox(box);
    QCOMPARE(boxFilter.topLeft(), QGeoCoordinate(30,30));
    QCOMPARE(boxFilter.boundingBox().topLeft(), QGeoCoordinate(30,30));
    QCOMPARE(boxFilter.bottomRight(), QGeoCoordinate(5,90));
    QCOMPARE(boxFilter.boundingBox().bottomRight(), QGeoCoordinate(5,90));

    QLandmarkCategoryFilter categoryFilter;
    QLandmarkCategory cat1;
    QLandmarkCategoryId catId1;
    catId1.setLocalId("1");
    catId1.setManagerUri("manager.uri");
    cat1.setCategoryId(catId1);

    categoryFilter.setCategory(cat1);
    QCOMPARE(categoryFilter.categoryId(), catId1);

    QLandmarkRemoveRequest lmRemoveRequest(m_manager);
    QLandmark lm1;
    QLandmarkId lmId1;
    lmId1.setLocalId("1");
    lmId1.setManagerUri("manager.uri");
    lm1.setLandmarkId(lmId1);

    QLandmark lm2;
    QLandmarkId lmId2;
    lmId2.setLocalId("2");
    lmId2.setManagerUri("manager.uri");
    lm2.setLandmarkId(lmId2);

    QLandmark lm3;
    QLandmarkId lmId3;
    lmId3.setLocalId("3");
    lmId3.setManagerUri("manager.uri");
    lm3.setLandmarkId(lmId3);

    lmRemoveRequest.setLandmark(lm1);
    QCOMPARE(lmRemoveRequest.landmarkIds().count(), 1);
    QCOMPARE(lmRemoveRequest.landmarkIds().at(0), lmId1);

    lmRemoveRequest.setLandmark(lm2);
    QCOMPARE(lmRemoveRequest.landmarkIds().count(), 1);
    QCOMPARE(lmRemoveRequest.landmarkIds().at(0), lmId2);

    QList<QLandmark> lms;
    lms << lm1 << lm2 << lm3;

    QList<QLandmarkId> lmIds;
    lmIds << lmId1 << lmId2 << lmId3;

    lmRemoveRequest.setLandmarks(lms);
    QCOMPARE(lmRemoveRequest.landmarkIds(), lmIds);

    lms.removeLast();
    lmIds.removeLast();
    lmRemoveRequest.setLandmarks(lms);
    QCOMPARE(lmRemoveRequest.landmarkIds(), lmIds);

    QLandmarkCategory cat2;
    QLandmarkCategoryId catId2;
    catId2.setLocalId("2");
    catId2.setManagerUri("manager.uri");
    cat2.setCategoryId(catId2);

    QLandmarkCategory cat3;
    QLandmarkCategoryId catId3;
    catId3.setLocalId("3");
    catId3.setManagerUri("manager.uri");
    cat3.setCategoryId(catId3);

    QLandmarkCategoryRemoveRequest catRemoveRequest(m_manager);
    catRemoveRequest.setCategory(cat1);
    QCOMPARE(catRemoveRequest.categoryIds().count(), 1);
    QCOMPARE(catRemoveRequest.categoryIds().at(0), catId1);

    catRemoveRequest.setCategory(cat2);
    QCOMPARE(catRemoveRequest.categoryIds().count(), 1);
    QCOMPARE(catRemoveRequest.categoryIds().at(0), catId2);

    QList<QLandmarkCategory> cats;
    cats << cat1 << cat2 << cat3;

    QList<QLandmarkCategoryId> catIds;
    catIds << catId1 << catId2 << catId3;

    catRemoveRequest.setCategories(cats);
    QCOMPARE(catRemoveRequest.categoryIds(), catIds);

    cats.removeLast();
    catIds.removeLast();
    catRemoveRequest.setCategories(cats);
    QCOMPARE(catRemoveRequest.categoryIds(), catIds);
}

QTEST_MAIN(tst_QLandmarkManager)
#include "tst_qlandmarkmanager.moc"

