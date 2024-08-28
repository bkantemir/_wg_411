#include "RollingStock.h"
#include "TheApp.h"
#include "TouchScreen.h"
#include "ScreenLine.h"

extern TheApp theApp;

float RollingStock::accelerationLinPassive = 0.005;
float RollingStock::accelerationLinActive = 0.01;
float RollingStock::accelerationOnDrag = 0.2;
float RollingStock::divorceSpeed = 0.8;
float RollingStock::maxTrainSpeed = 3;
float RollingStock::maxDragSpeed = 3;

int RollingStock::onDeployRS(RollingStock* pRS, std::string tagStr) {
    SceneSubj::onDeployStandatd(pRS, tagStr);
    if (pRS->d2parent != 0)
        return 0;
    //find couplers
    for (int elN = 1; elN < pRS->totalElements; elN++) {
        SceneSubj* pSS = pRS->pSubjsSet->at(pRS->nInSubjsSet + elN);
        if (pSS == NULL)
            continue;
        if (strcmp(pSS->className, "coupler") == 0) {
            if (pSS->alignedWithRoot > 0)
                pRS->pCouplerFront = (Coupler*)pSS;
            else
                pRS->pCouplerBack = (Coupler*)pSS;
        }
        else if (strcmp(pSS->className, "gangway") == 0) {
            if (pSS->alignedWithRoot > 0)
                pRS->pGangwayFront = (Gangway*)pSS;
            else
                pRS->pGangwayBack = (Gangway*)pSS;
        }
    }
    //train info
    RollingStock::reinspectTrain(pRS->pCouplerFront);
    //place to nearest rail
    RailCoord* pRC=&pRS->railCoord;
                                pRC->railN = -1;
    if (pRC->railN < 0) {
        int rN = theApp.gameTable.findClosestRail(pRC, &pRS->ownCoords);
        if (rN < 0) //no rail found
            return -1;
        //populate to ownCoords
        v3copy(pRS->ownCoords.pos, pRC->xyz);
        pRS->ownCoords.setEulerDg(0, pRC->currentYaw, 0);
     }
    return 1;
}

int RollingStock::onLoadRS(RollingStock* pSS00, std::string tagStr) {
    if (pSS00->d2parent != 0)
        return 0;
    //inspect RS
    pSS00->wheelBase = 0;
    pSS00->wheelBaseZ = 0;
    pSS00->zOffsetFromRoot = 0;
    pSS00->alignedWithRoot = 1;
    for (int elN = 1; elN < pSS00->totalElements; elN++) {
        SceneSubj* pSS = pSS00->pSubjsSet->at(pSS00->nInSubjsSet + elN);
        if (pSS == NULL)
            continue;
        SceneSubj::buildModelMatrixStandard(pSS);
        pSS->zOffsetFromRoot = pSS->absCoords.pos[2];

        float vIn[4] = { 0,0,1,0 };
        float vOut[4];
        mat4x4_mul_vec4plus(vOut, pSS->absModelMatrixUnscaled, vIn, 0);
        if(vOut[2]<0)
            pSS->alignedWithRoot = -1;
        else
            pSS->alignedWithRoot = 1;

        if (strcmp(pSS->className, "coupler") == 0) {
            if (pSS->absCoords.pos[1] != Rail::couplerY) {
                float dy = Rail::couplerY - pSS->absCoords.pos[1];
                pSS->ownCoords.pos[1] += dy;
            }
            if (pSS->alignedWithRoot > 0)
                pSS00->pCouplerFront = (Coupler*)pSS;
            else
                pSS00->pCouplerBack = (Coupler*)pSS;
        }
    }
    pSS00->unitLength = pSS00->pCouplerFront->zOffsetFromRoot - pSS00->pCouplerBack->zOffsetFromRoot +
        pSS00->pCouplerFront->lever + pSS00->pCouplerBack->lever;
    pSS00->maxSectionLength = pSS00->unitLength;
    return 1;
}

bool RollingStock::ignoreCollisionRollingStock(RollingStock* pRS1, SceneSubj* pS2) {
    if (strstr(pS2->className, "RollingStock") != pS2->className)
        return false;
    //dealing with RollingStock, check if coupled with it
    Coupler* pCp = pRS1->pCouplerFront;
    if (pCp != NULL)
        if (pCp->pCounterCoupler != NULL) {
            Coupler* pCp2 = pCp->pCounterCoupler;
            if (pCp2->rootN == pS2->rootN)
                return true;
        }
    pCp = pRS1->pCouplerBack;
    if (pCp != NULL)
        if (pCp->pCounterCoupler != NULL) {
            Coupler* pCp2 = pCp->pCounterCoupler;
            if (pCp2->rootN == pS2->rootN)
                return true;
        }
    return false;
}
int RollingStock::processCollisionRollingStock(float penetrationDepth, RollingStock* pS1, SceneSubj* pS2, float* collisionPointWorld, float* hitPointNormal, float* hitSpeed) {
    if (strstr(pS2->className, "RollingStock") == pS2->className) {
        if (pS1->nInSubjsSet > pS2->nInSubjsSet)
            return 0;
        divorceTrains(pS1, (RollingStock*)pS2);
        return 1;
    }
    float pushDir[4];
    if (pS1->nInSubjsSet > pS2->nInSubjsSet) { //and if the same rail
        //take opposite dir to pS2
        mat4x4_mul_vec4plus(pushDir, pS2->absCoords.rotationMatrix, pS2->ownSpeed.pos, 0, false);
        for (int i = 0; i < 3; i++)
            pushDir[i] = -pushDir[i];
    }
    else {
        for (int i = 0; i < 3; i++)
            pushDir[i] = pS1->absCoords.pos[i] - pS2->absCoords.pos[i];
    }
    v3norm(pushDir);
    //calculate wagon's z-direction
    float zDir[4];
    float vIn[4] = { 0,0,1,0 };
    mat4x4_mul_vec4plus(zDir, pS1->absCoords.rotationMatrix, vIn, 0,false);
    float dotP = v3dotProduct(pushDir, zDir);
    int alignedWithZ = 0; 
    if (dotP > 0)
        alignedWithZ = 1;
    else
        alignedWithZ = -1;
    float shift = 0.5*alignedWithZ;
    emergencyTrainShift(pS1, shift,(RollingStock*)pS2);
    return 1;
}

int RollingStock::checkTrains(std::vector<SceneSubj*>* pSceneSubjs) {
    int subjsN = pSceneSubjs->size();
    for (int sN = 0; sN < subjsN; sN++) {
        SceneSubj* pS = pSceneSubjs->at(sN);
        if (pS == NULL)
            continue;
        if (pS->d2parent != 0)
            continue;
        if (strstr(pS->className, "RollingStock") != pS->className)
            continue;
        RollingStock* pRS = (RollingStock*)pS;
        pRS->powerOn = false;
        if (pRS->trainRootN != pRS->nInSubjsSet)
            continue;
        Train* pTr = &pRS->tr;
        couplerClearance(pTr->pCouplerFront);
        couplerClearance(pTr->pCouplerBack);
    }
    //turn lights on
    int primeLocoN = 0;
    RollingStock* pRS = (RollingStock*)pSceneSubjs->at(primeLocoN);
    if (pRS->powered) {
        pRS->powerOn = true;
        Coupler* pCp = pRS->pCouplerFront;
        while (pCp->connected > 0 && pCp->couplersInvolved > 1) {
            pCp = pCp->pCounterCoupler;
            pRS = (RollingStock*)pSceneSubjs->at(pCp->rootN);
            pRS->powerOn = true;
            //next coupler
            if (pCp == pRS->pCouplerFront)
                pCp = pRS->pCouplerBack;
            else
                pCp = pRS->pCouplerFront;
        }
        pRS = (RollingStock*)pSceneSubjs->at(primeLocoN);
        pCp = pRS->pCouplerBack;
        while (pCp->connected > 0 && pCp->couplersInvolved > 1) {
            pCp = pCp->pCounterCoupler;
            pRS = (RollingStock*)pSceneSubjs->at(pCp->rootN);
            pRS->powerOn = true;
            //next coupler
            if (pCp == pRS->pCouplerFront)
                pCp = pRS->pCouplerBack;
            else
                pCp = pRS->pCouplerFront;
        }
    }
    return 1;
}
float RollingStock::couplerClearance(Coupler* pCoupler) {
    return 100;
}
int RollingStock::readClassPropsRS(RollingStock* pRS, std::string tagStr) {
    if (tagStr.length() == 0)
        return 0;

    if (XMLparser::varExists("loco", tagStr))
        pRS->powered = 1;

    return 1;
}

int RollingStock::moveRS(RollingStock* pRS) {

    RollingStock* pRSroot = (RollingStock*)pRS->pSubjsSet->at(pRS->trainRootN);
    Train* pTr = &pRSroot->tr;
    if (pRS->trainRootN == pRS->nInSubjsSet) {
        //is train root
        //check if dragged
        if (TouchScreen::capturedCode.compare("ROLLINGSTOCK")==0) {
            RollingStock* pDraged = (RollingStock*)pSelectedSceneSubj;
            if (pDraged->trainRootN == pRS->trainRootN) {
                return 0;
            }
        }
        if (pTr->poweredLength == 0) {
            pRSroot->desirableZdir = 0;
            pRSroot->desirableZspeed = 0;
            pRSroot->accelerationLin = RollingStock::accelerationLinPassive;
        }
        else { // powered root
            pRSroot->accelerationLin = RollingStock::accelerationLinActive;
            /*
            //pick desirable speed from prime loco
            RollingStock* pLoco = (RollingStock*)pRS->pSubjsSet->at(pTr->primeLocoN);
            pRS->desirableZdir = pLoco->desirableZdir * pRS->alignedWithTrainHead * pLoco->alignedWithTrainHead;
            */

            if(obstacleAhead(pRSroot))
                pRS->desirableZspeed = 0;
            else
                pRS->desirableZspeed = pRS->desirableZdir * RollingStock::maxTrainSpeed;

            //mylog("pRS->desirableZspeed=%f pRS->ownZspeed=%f\n", pRS->desirableZspeed, pRS->ownZspeed);
        }
        if (pRS->ownZspeed == 0 && pRS->desirableZspeed == 0)
            return 0;
        applySpeedOnRails(pRS);
        pTr->divorcing = false;
    }
    else {//follow root
        float shiftFromRoot = (pRSroot->zOffsetFromHead- pRS->zOffsetFromHead)* pRSroot->alignedWithTrainHead;
        theApp.gameTable.shiftRailCoord(&pRS->railCoord, &pRSroot->railCoord, &theApp.gameTable, shiftFromRoot);
        if (pRS->alignedWithTrainHead != pRSroot->alignedWithTrainHead)
            pRS->railCoord.alignedWithRail *= -1;
    }
    //calc ownSpeed
    for (int i = 0; i < 3; i++)
        pRS->ownSpeed.pos[i] = pRS->railCoord.xyz[i] - pRS->railCoordOld.xyz[i];
    return 1;
}
int RollingStock::emergencyTrainShift(RollingStock* pW1, float divorceDir, RollingStock* pW2) {
    RollingStock* pRoot1 = (RollingStock*)pW1->pSubjsSet->at(pW1->trainRootN);
    RollingStock* pRoot2 = (RollingStock*)pW2->pSubjsSet->at(pW2->trainRootN);
    Train* pTr1 = &pRoot1->tr;
    Train* pTr2 = &pRoot2->tr;
    if (pTr1->divorcing) { //already divorcing
        return 0;
    }
    //drop tail
    Coupler* pCp2free = pW1->pCouplerBack;
    if(divorceDir<0)
        pCp2free = pW1->pCouplerFront;
    if (pCp2free->connected > 0) {
        Coupler* pCp3 = pCp2free->pCounterCoupler;
        pCp3->connected = 0;
        reinspectTrain(pCp3);
        RollingStock* pW3 = (RollingStock*)pCp3->pSubjsSet->at(pCp3->rootN);
        RollingStock* pRoot3 = (RollingStock*)pW2->pSubjsSet->at(pW3->trainRootN);
        pRoot3->ownZspeed = 0;

        pCp2free->connected = 0;
        reinspectTrain(pCp2free);
        pRoot1 = (RollingStock*)pW1->pSubjsSet->at(pW1->trainRootN);
        pTr1 = &pRoot1->tr;
    }
    if (pW1->alignedWithTrainHead != pRoot1->alignedWithTrainHead)
        divorceDir *= -1;

     pRoot1->tr.divorcing = true;
     pRoot1->ownZspeed = divorceDir*divorceSpeed;
     /*
     {
         mylog("emergencyTrainShift car%d:%d->root %d:%d->train %d dir %d\n",pW1->nInSubjsSet,pW1->alignedWithTrainHead, 
             pRoot1->nInSubjsSet, pRoot1->alignedWithTrainHead,pTr1->trainId, (int)divorceDir);
     }
     */
     return 1;
}
int RollingStock::trainToLog(Coupler* pCp) {
    RollingStock* pW = (RollingStock*)pCp->pSubjsSet->at(pCp->rootN);
    return trainToLog(pW);
}
int RollingStock::trainToLog(RollingStock* pW0) {
    RollingStock* pRoot = (RollingStock*)pW0->pSubjsSet->at(pW0->trainRootN);
    mylog("%d.%s:rootN %d, units %d: ", pW0->nInSubjsSet, path2title(pW0->source256).c_str(), pW0->trainRootN, pRoot->tr.carsTotal);
    Coupler* pCp = pRoot->tr.pCouplerFront;
    int carsN = 0;
    while (1) {
        RollingStock* pW = (RollingStock*)pRoot->pSubjsSet->at(pCp->rootN);
        if (pW->alignedWithTrainHead > 0)
            mylog("+");
        else
            mylog("-");
        mylog("%d.%s:%d", pW->nInSubjsSet,path2title(pW->source256).c_str(),(int)pW->zOffsetFromHead);
        if (pW == pW0)
            mylog("!!");
        carsN++;
        //to next coupler
        if (pCp->alignedWithRoot > 0)
            pCp = pW->pCouplerBack;
        else
            pCp = pW->pCouplerFront;
        if (pCp->connected < 1)
            break;//tail reached
        //go to next car
        pCp = pCp->pCounterCoupler;
        mylog(" -> ");
    }
    mylog("\n");
    if (carsN != pRoot->tr.carsTotal) {
        mylog("ERROR: not declared cars N. Slow-mo:\n");
        Coupler* pCp = pRoot->tr.pCouplerFront;
        int carsN = 0;
        while (1) {
            mylog("checking coupler %d:%d\ncar:", pCp->rootN, pCp->alignedWithRoot);
            RollingStock* pW = (RollingStock*)pRoot->pSubjsSet->at(pCp->rootN);
            if (pW->alignedWithTrainHead > 0)
                mylog("+");
            else
                mylog("-");
            mylog("%d.%s:%d", pW->nInSubjsSet, path2title(pW->source256).c_str(), (int)pW->zOffsetFromHead);
            if (pW == pW0)
                mylog("!!");
            mylog("\n");
            carsN++;
            //to next coupler
            if (pCp->alignedWithRoot > 0)
                pCp = pW->pCouplerBack;
            else
                pCp = pW->pCouplerFront;
            mylog("opposite coupler %d:%d\n", pCp->rootN, pCp->alignedWithRoot);
            if (pCp->connected < 1) {
                mylog("The last one\n");
                break;//tail reached
            }
            //go to next car
            pCp = pCp->pCounterCoupler;
            mylog("connected to %d:%d\n", pCp->rootN, pCp->alignedWithRoot);
        }
        int a = 0;
    }
    return 1;
}
std::string RollingStock::path2title(std::string path) {
    int lastSlashPos = path.find_last_of("/");
    std::string path2 = path.substr(0, lastSlashPos);
    lastSlashPos = path2.find_last_of("/");
    std::string name = path2.substr(lastSlashPos+1);
    return name;
}
bool RollingStock::trainRootIsWrong(RollingStock* pW0) {
    RollingStock* pRoot = (RollingStock*)pW0->pSubjsSet->at(pW0->trainRootN);
    Coupler* pCp = pRoot->tr.pCouplerFront;
    while (1) {
        RollingStock* pW = (RollingStock*)pCp->pSubjsSet->at(pCp->rootN);
        if (pW->nInSubjsSet == pW0->nInSubjsSet)
            return false;
        //switch to opposite coupler
        if (pCp->alignedWithRoot > 0)
            pCp = pW->pCouplerBack;
        else
            pCp = pW->pCouplerFront;
        //to next car
        if (pCp->connected < 1)
            return true;//tail reached
        pCp = pCp->pCounterCoupler;
    }
}
int RollingStock::reinspectTrain(Coupler* pCoupler) {
    SceneSubj* pSS = pCoupler->pSubjsSet->at(pCoupler->rootN);
    if (strstr(pSS->className, "RollingStock") != pSS->className)
        return 0;//not a wagon
    pCoupler = findHeadCoupler(pCoupler);
    return assignNewTrainHead(pCoupler);
}
Coupler* RollingStock::findHeadCoupler(Coupler* pCp) {
    while (pCp->connected > 0) {
 
        //mylog("findHeadCoupler checking Coupler %d:%d\n", pCp->rootN,pCp->alignedWithRoot);

        //coupler connected
        Coupler* pCpConnected = pCp->pCounterCoupler;
 
        //mylog("findHeadCoupler connected to %d:%d\n", pCpConnected->rootN, pCpConnected->alignedWithRoot);
        
        RollingStock* pWagonNext = (RollingStock*)pCpConnected->pSubjsSet->at(pCpConnected->rootN);

        //proceed to opposite coupler
        if (pCpConnected->alignedWithRoot > 0)
            pCp = pWagonNext->pCouplerBack;
        else
            pCp = pWagonNext->pCouplerFront;

        //mylog("findHeadCoupler proceed to nextCoupler %d:%d\n", pCp->rootN, pCp->alignedWithRoot);
    }
    //mylog("findHeadCoupler . Head coupler %d:%d\n", pCp->rootN, pCp->alignedWithRoot);

    return pCp;
}
int RollingStock::assignNewTrainHead(Coupler* pCp) {
    //pCp-train front coupler
    if(pCp->connected > 0) {
        mylog("ERROR in RollingStock::assignNewTrainHead: not a head coupler\n");
        return -1;
    }
    Train tr;
    int trainRootN = pCp->rootN;
    tr.trainId = newId();
    tr.primeLocoN = -1;
    tr.carsTotal = 0;
    tr.pCouplerFront = pCp;
    float trainLength = 0;
    float poweredLength = 0;
    //just in case
    tr.divorcing = false;

    //mylog("assignNewTrainHead set HEAD coupler %d.%d\n", pCp->rootN, pCp->alignedWithRoot);

    std::vector<int> trainWagonNs;
    float zOffsetFromHead = 0;
    int alignedWithTrainHead = pCp->alignedWithRoot;
    RollingStock* pWagon = NULL;
    while(1){

        //mylog("assignNewTrainHead coupler %d.%d\n", pCp->rootN, pCp->alignedWithRoot);

        pWagon = (RollingStock*)pCp->pSubjsSet->at(pCp->rootN);
        //count current wagon
        pWagon->nInTrain = trainWagonNs.size();
        trainWagonNs.push_back(pWagon->nInSubjsSet);
        pWagon->alignedWithTrainHead = alignedWithTrainHead;
        pWagon->zOffsetFromHead = zOffsetFromHead;
        if (trainRootN > pCp->rootN)
            trainRootN = pCp->rootN;
        if (pWagon->activeLoco) {
            tr.primeLocoN = pWagon->nInSubjsSet;
            tr.poweredLength = pWagon->unitLength;
        }
        //move to opposite coupler
        if (pCp->alignedWithRoot > 0)
            pCp = pWagon->pCouplerBack;
        else
            pCp = pWagon->pCouplerFront;

        //mylog("assignNewTrainHead opposite coupler %d.%d\n", pWagon->nInSubjsSet, pCp->alignedWithRoot);

        zOffsetFromHead += abs(pCp->zOffsetFromRoot);
        if (pCp->connected < 1) {
            //mylog("That's the last coupler\n");
            //it's a last coupler
            break;
        }
        //if here - coupler connected
        //add dist between wagons
        zOffsetFromHead += Coupler::couplingDistances[pCp->couplersInvolved];
        zOffsetFromHead += pCp->lever;
        Coupler* pCpConnected = pCp->pCounterCoupler;
        if (pCp->alignedWithRoot == pCpConnected->alignedWithRoot)
            alignedWithTrainHead *= -1;
        //proceed to connected coupler
        if (pCpConnected->pCounterCoupler != pCp) {
            Coupler* pCp2 = pCpConnected->pCounterCoupler;
            mylog("Wrong cross-connection: %d:%d to %d:%d when %d:%d to %d:%d\n", 
                pCp->rootN, pCp->alignedWithRoot, pCpConnected->rootN, pCpConnected->alignedWithRoot,
                pCpConnected->rootN, pCpConnected->alignedWithRoot, pCp2->rootN, pCp2->alignedWithRoot);
            return -1;
        }

        pCp = pCpConnected;
        zOffsetFromHead += abs(pCp->zOffsetFromRoot);
        zOffsetFromHead += pCp->lever;
        /*
        //mylog("assignNewTrainHead proceed to connected coupler %d.%d\n", pCp->rootN, pCp->alignedWithRoot);

        if (trainWagonNs.size() > 10) {
            mylog("Short cirquit\n");
            trainToLog(pWagon);
            int a = 0;
        }
        */
    }
    tr.pCouplerBack = pCp;
    tr.carsTotal = trainWagonNs.size();
    tr.trainLength = zOffsetFromHead;
    //reassign trainRoot
    //mylog("assignNewTrainHead assigning new root %d to %d/%d wagons:", trainRootN, trainWagonNs.size(), tr.carsTotal);
    for (int wN = 0; wN < tr.carsTotal; wN++) {
        pWagon = (RollingStock*)pWagon->pSubjsSet->at(trainWagonNs.at(wN));
        //mylog(" %d,", pWagon->nInSubjsSet);
        pWagon->trainRootN = trainRootN;
    }
    //mylog("\n");

    RollingStock* pRoot = (RollingStock*)pWagon->pSubjsSet->at(trainRootN);
    memcpy(&pRoot->tr, &tr, sizeof(Train));
    if (tr.primeLocoN >= 0 && tr.primeLocoN != trainRootN){
        RollingStock* pLoco = (RollingStock*)pWagon->pSubjsSet->at(tr.primeLocoN);
        pRoot->desirableZdir = pLoco->desirableZdir;
        if (pRoot->alignedWithTrainHead != pLoco->alignedWithTrainHead)
            pRoot->desirableZdir *= -1;
    }
    //mylog("%d wagons\n", tr.carsTotal);
    return tr.carsTotal;
}
int RollingStock::divorceTrains(RollingStock* pW1, RollingStock* pW2) {
    /*
    {
        mylog("---%d divorceTrains:\n", (int)theApp.frameN);
        trainToLog(pW1);
        trainToLog(pW2);
        mylog("Solution:\n");
    }
    */
    if (pW1->trainRootN == pW2->trainRootN) {
        //same train
        //mylog("Same train, skip:\n");
        return 1;
    }
    float dir2to1[4];
    v3dirFromTo(dir2to1, pW2->absCoords.pos, pW1->absCoords.pos);
    float vIn[4] = { 0,0,1,0 };
    float r1dir[4];
    mat4x4_mul_vec4plus(r1dir, pW1->ownCoords.rotationMatrix, vIn, 0);
    if (v3length(dir2to1)<0.5)
        v3copy(dir2to1, r1dir);
    float dot1 = v3dotProductNorm(dir2to1, r1dir);
    float w1shift = 1;
    if (dot1 < 0)
        w1shift *= -1;
    emergencyTrainShift(pW1, w1shift, pW2);

    float dir1to2[4];
    for (int i = 0; i < 3; i++)
        dir1to2[i] = -dir2to1[i];
    float r2dir[4];
    mat4x4_mul_vec4plus(r2dir, pW2->ownCoords.rotationMatrix, vIn, 0);
    float dot2 = v3dotProductNorm(dir1to2, r2dir);
    if (abs(dot2) > 0.3) {
        float w2shift = 1;
        if (dot2 < 0)
            w2shift *= -1;
        emergencyTrainShift(pW2, w2shift, pW1);
    }
    return 1;
}
int RollingStock::decouple(Coupler* pCp2free) {
    if (pCp2free->connected < 1)
        return 0;
    std::vector<SceneSubj*>* pSubjs = pCp2free->pSubjsSet;
    RollingStock* pWg00 = (RollingStock*)pSubjs->at(pCp2free->rootN);
    RollingStock* pRoot00 = (RollingStock*)pSubjs->at(pWg00->trainRootN);
    //involved couplers
    Coupler* pCps[2];
    pCps[0] = pCp2free;
    pCps[1] = pCp2free->pCounterCoupler;
    for (int wN = 0; wN < 2; wN++) {
        Coupler* pCp=pCps[wN];
        RollingStock* pWg = (RollingStock*)pSubjs->at(pCp->rootN);
        int alignedWithRoot00 = pWg->alignedWithTrainHead * pRoot00->alignedWithTrainHead;
        pCp->connected = 0;
        reinspectTrain(pCp);
        if (pWg->trainRootN == pRoot00->nInSubjsSet)
            continue;
        //new root
        RollingStock* pRoot = (RollingStock*)pSubjs->at(pWg->trainRootN);
        int alignedRoots = pWg->alignedWithTrainHead * pRoot->alignedWithTrainHead * alignedWithRoot00;
        pRoot->desirableZdir = pRoot00->desirableZdir * alignedRoots;
        pRoot->ownZspeed = pRoot00->ownZspeed * alignedRoots;
    }
    return 1;
}
bool RollingStock::isDraggable() {
    return true;
    /*
    RollingStock* pRS = this;
    if (pRS->powered)
        return true;
    return false;
    */
}

bool RollingStock::obstacleAhead(RollingStock* pTrainRoot) {
    if (pTrainRoot->nInSubjsSet != pTrainRoot->trainRootN)
        pTrainRoot = (RollingStock*)pTrainRoot->pSubjsSet->at(pTrainRoot->trainRootN);
    if(pTrainRoot->ownZspeed==0)
        return false;
    // s = (v0 * v0) / (a * 2)
    float brakingDistance = (pTrainRoot->ownZspeed * pTrainRoot->ownZspeed) / (pTrainRoot->accelerationLin * 2);
    Train* pTr = &pTrainRoot->tr;
    //forward coupler
    Coupler* pFwdCp = pTr->pCouplerFront;
    RollingStock* pFwdCar = (RollingStock*)pFwdCp->pSubjsSet->at(pFwdCp->rootN);
    if(pFwdCar->alignedWithTrainHead * pTrainRoot->alignedWithTrainHead * pTrainRoot->ownZspeed < 0)
        pFwdCp = pTr->pCouplerBack;
    //check clearance
    if (pFwdCp->connected >= 0)
        if (pFwdCp->distance <= brakingDistance)
            return true;

    return false;
}
int RollingStock::onDragRS(RollingStock* pRS) {
    RollingStock* pRoot = (RollingStock*)pRS->pSubjsSet->at(pRS->trainRootN);

    // blinking?
    if(pRoot->ownZspeed!=0)
        pRS->setHighLight(0);
    else {
        int cycle = 20;
        if ((int)(theApp.frameN - TouchScreen::captureFrameN) % cycle == 0)
            pRS->setHighLight(0.4, MyColor::getUint32(1.0f, 1.0f, 1.0f));
        else
            pRS->setHighLight(0);
    }

    //drag direction
    mat4x4 mMVP;
    mat4x4_mul(mMVP, theApp.mainCamera.mViewProjection, pSelectedSceneSubj00->absModelMatrix);
    float* targetRads = theApp.mainCamera.targetRads;
    if (mat4x4_mul_vec4screen(TouchScreen::ancorPointOnScreen, mMVP, TouchScreen::ancorPoint, targetRads) < 0) {
        //point out of reach - drop drag
        pSelectedSceneSubj = NULL;
        pSelectedSceneSubj00 = NULL;
        TouchScreen::pSelected = NULL;
        TouchScreen::capturedCode.assign("NONE");
        TouchScreen::cursorStatus = 0;
        return 0;        
    }

    float dragDir[2];
    for (int i = 0; i < 2; i++)
        dragDir[i] = TouchScreen::lastCursorPos[i] - TouchScreen::ancorPointOnScreen[i];
    float wagonDir[2];
    for (int i = 0; i < 2; i++)
        wagonDir[i] = pRS->pCouplerFront->gabaritesOnScreen.bbMid[i] - pRS->pCouplerBack->gabaritesOnScreen.bbMid[i];
    float shiftDir=0;//stop train
    float shiftDistance = 0;
    if (v3lengthXY(dragDir) > 3) {
        if (v3lengthXY(wagonDir) > 3) {
            float dp = v2dotProduct(wagonDir, dragDir);
            if (abs(dp) > 0.1) {
                shiftDistance = dp * v3lengthXY(dragDir);
                if (abs(shiftDistance) > 10) {
                    shiftDir = signOf(shiftDistance);
                }
            }
        }
    }
    pRoot->desirableZdir = shiftDir * pRoot->alignedWithTrainHead * pRS->alignedWithTrainHead;
    pRoot->desirableZspeed = 0;
    pRoot->accelerationLin = accelerationOnDrag;
    if (!obstacleAhead(pRoot)){
        float brakingDistance = (pRoot->ownZspeed * pRoot->ownZspeed) / pRoot->accelerationLin;
        float sizeUnitPixelsSize = getUnitPixelsSize(pRS, &theApp.mainCamera);
        float brakingDistancePx = brakingDistance * sizeUnitPixelsSize;
        if (abs(shiftDistance) <= brakingDistancePx)
            pRoot->desirableZspeed = 0;
        else
            pRoot->desirableZspeed = pRoot->desirableZdir * RollingStock::maxDragSpeed;
    }
    if (pRoot->ownZspeed != 0 || pRoot->desirableZspeed != 0) {
        applySpeedOnRails(pRoot);
    }
    //add drag ribbon
    if (v3lengthFromToXY(TouchScreen::ancorPointOnScreen, TouchScreen::lastCursorPos) > 30) {
        float dp = abs(v2dotProduct(wagonDir, dragDir));
        if (dp < 0.1) {
            if (v3lengthXY(dragDir) > UISubj::screenSize[1] * 0.15) {
                shiftDir = 0;
                TouchScreen::leftButtonUp();
                return 0;
            }
        }
        float g = fmin(1.0, dp * 2);
        float r = fmin(1.0, (1.0 - dp) * 2);
        MyColor color;
        color.setRGBA(r, g, 0.0f, 0.5f);

        float ribbonL = v3lengthXY(dragDir);
        float screenHpercent = ribbonL / UISubj::screenSize[1];
        float maxAllowedPercent = 0.6;
        if (screenHpercent> maxAllowedPercent) {
            shiftDir = 0;
            TouchScreen::leftButtonUp();
            return 0;
        }
        float ribbonW = 10.0 * (1.0- screenHpercent/maxAllowedPercent );
        if (ribbonW < 1.0)
            ribbonW = 1.0;
        ScreenLine::addLine2queue(TouchScreen::ancorPointOnScreen, TouchScreen::lastCursorPos, color.getUint32(), ribbonW, true);
        /*
        {//debug
            mylog("%d ancor[ %d x %d ]. ",(int)theApp.frameN,(int)TouchScreen::ancorPointOnScreen[0], (int)TouchScreen::ancorPointOnScreen[1]);
            mylog("R=%d ",(int)pSelectedSceneSubj00->gabaritesOnScreen.chordR);
            float gl[4];
            mat4x4_mul_vec4plus(gl, theApp.mainCamera.mViewProjection, pSelectedSceneSubj00->absCoords.pos,1, true);
            mylog("GL[ %f x %f x %f x %f ]\n", gl[0], gl[1], gl[2], gl[3]);
        }
    */
    }
    return 1;
}
int RollingStock::onLeftButtonDown() {
    SceneSubj::onLeftButtonDown();
    TouchScreen::capturedCode.assign("ROLLINGSTOCK");
    return 1;
}
