#include "Actor.h"
#include "StudentWorld.h"
#include <list>
#include <iostream>

//-----------------------------------------------------------------------------------------

//Actor

Actor::Actor(int imageID, double xc, double yc, int ddir, int ddepth, StudentWorld* w)
: GraphObject(imageID, xc, yc, ddir, ddepth), alive(true), m_hell(w){}

bool Actor::canBeDamaged(){
    return false;
}

bool Actor::canBeInfected(){
    return false;
}

bool Actor::blockMove(){
    return false;
}

bool Actor::blockFlame(){
    return false;
}

bool Actor::isAlive(){
    return alive;
}

void Actor::setDead(){
    alive = false;
}

StudentWorld* Actor::hell(){
    return m_hell;
}

//-----------------------------------------------------------------------------------------

//Wall and Exit: Two Special Blocks Directly Derived From Actor

Wall::Wall(int xc, int yc, StudentWorld* w) : Actor(IID_WALL, xc, yc, 0, 0, w){}

bool Wall::blockMove(){
    return true;
}

bool Wall::blockFlame(){
    return true;
}

int Wall::doSomething(){return GWSTATUS_CONTINUE_GAME;}

Exit::Exit(double xc, double yc, StudentWorld* w):Actor(IID_EXIT, xc, yc, 0, 1, w){}

bool Exit::blockFlame(){
    return true;
}

/*
 void Exit::doSomething(){
 list<Actor*>::iterator pp = hell()->overlapCitizen(this);
 hell()->remove(pp);
 }
 */

int Exit::doSomething(){
    return hell()->exitDo(this);
}

//-----------------------------------------------------------------------------------------

//First Level Virtual Classes Derived From Actor

CanBeDamaged::CanBeDamaged(int imageID, double xc, double yc, int ddir, int ddepth, StudentWorld* w)
: Actor(imageID, xc, yc, ddir, ddepth, w){}

bool CanBeDamaged::canBeDamaged(){
    return true;
}


Prop::Prop(int imageID, double xc, double yc, int ddir, StudentWorld* w)
: Actor(imageID, xc, yc, ddir, 0, w){}

//-----------------------------------------------------------------------------------------

//Second Level Virtual Classes

People::People(int imageID, double xc, double yc, StudentWorld* w)
: CanBeDamaged(imageID, xc, yc, 0, 0, w){}

void People::newCoord(double& x, double& y, int direction, double amount){
    if(direction==left)
        x-=amount;
    if(direction==right)
        x+=amount;
    if(direction==up)
        y+=amount;
    if(direction==down)
        y-=amount;
}

int People::approach(double targetX, double targetY, int& another){
    double selfX = getX();
    double selfY = getY();
    if(selfX==targetX){
        if(selfY>targetY)
            return down;
        else
            return up;
    }
    if(selfY==targetY) {
        if(selfX>targetX)
            return left;
        else
            return right;
    }
    int n = randInt(1,2);
    if(selfX>targetX){
        if(selfY>targetY){
            switch(n){
                case 1:
                    another = down;
                    return left;
                default:
                    another = left;
                    return down;
            }
        } else {
            switch(n){
                case 1:
                    another = up;
                    return left;
                default:
                    another = left;
                    return up;
            }
        }
    } else {
        if(selfY>targetY){
            switch(n){
                case 1:
                    another = down;
                    return right;
                default:
                    another = right;
                    return down;
            }
        } else {
            switch(n){
                case 1:
                    another = up;
                    return right;
                default:
                    another = right;
                    return up;
            }
        }
    }
}

GoodPeople::GoodPeople(int imageID, double xc, double yc, StudentWorld* w)
: People(imageID, xc, yc, w){
    infected = false;
    infectionCount = 0;
}

void GoodPeople::infect(){
    infected = true;
}

bool GoodPeople::isInfected(){
    return infected;
}

bool GoodPeople::getWorse(){
    if(infected){
        infectionCount++;
        return true;
    }
    return false;
}

int GoodPeople::getInfectionCount(){
    return infectionCount;
}

bool GoodPeople::canBeInfected(){
    return true;
}

bool GoodPeople::infectToDeath(){
    if(isInfected()){
        getWorse();
        if(getInfectionCount()>=500){
            return true;
        }
    }
    return false;
}

bool GoodPeople::cure(){
    if(infected){
        infected = false;
        return true;
    }
    return false;
}

Goodie::Goodie(int imageID, double xc, double yc, StudentWorld* w)
: CanBeDamaged(imageID, xc, yc, 0, 1, w){}

BadPeople::BadPeople(int imageID, double xc, double yc, StudentWorld* w)
: People(imageID, xc, yc, w), paralyzed(false), movementPlan(0){}

bool BadPeople::planMovement(){
    if(movementPlan!=0)
        return false;
    int n = randInt(3,10);
    movementPlan = n;
    return true;
}

bool BadPeople::zombieMove(){
    int direction = getDirection();
    double dest_x = getX();
    double dest_y = getY();
    newCoord(dest_x, dest_y, direction, 1);
    if(hell()->canMoveTo(dest_x, dest_y, this)){
        moveTo(dest_x,dest_y);
        movementPlan--;
        return true;
    } else {
        movementPlan=0;
        return false;
    }
}

void BadPeople::setRandomDirection(){
    int i = randInt(1,4);
    switch(i){
        case 1:
            setDirection(up);
            break;
        case 2:
            setDirection(down);
            break;
        case 3:
            setDirection(left);
            break;
        case 4:
            setDirection(right);
            break;
    }
}

bool BadPeople::vomit(){
    double vomitX = getX();
    double vomitY = getY();
    int direction = getDirection();
    if(direction==left||direction==right)
        newCoord(vomitX, vomitY, direction, SPRITE_WIDTH);
    else
        newCoord(vomitX, vomitY, direction, SPRITE_HEIGHT);
    if(hell()->personInFront(vomitX, vomitY)){
        int n = randInt(1,3);
        if(n==1){
            Vomit* v = new Vomit(vomitX, vomitY, getDirection(), hell());
            hell()->addActor(v);
            hell()->playSound(SOUND_ZOMBIE_VOMIT);
            changeParalyzeStatus();
            return true;
        }
    }
    return false;
}

void BadPeople::damage(){
    hell()->playSound(SOUND_ZOMBIE_DIE);
    hell()->increaseScore(2000);
    setDead();
}

/*
bool BadPeople::decMovementPlan(){
    if(movementPlan<=0)
        return false;
    movementPlan--;
    return true;
}
 */

//-----------------------------------------------------------------------------------------

//Prop Classes

Pit::Pit(double xc, double yc, StudentWorld* w)
: Prop(IID_PIT, xc, yc, 0, w){}

int Pit::doSomething(){
    hell()->burn(this);
    
    return GWSTATUS_CONTINUE_GAME;
}

Flame::Flame(double xc, double yc, int ddir, StudentWorld* w)
: Prop(IID_FLAME, xc, yc, ddir, w), ttl(2){}

int Flame::doSomething(){
    if(!isAlive())
        return GWSTATUS_CONTINUE_GAME;
    if(ttl==0){
        setDead();
        return GWSTATUS_CONTINUE_GAME;
    }
    hell()->burn(this);
    ttl--;
    return GWSTATUS_CONTINUE_GAME;
}

Vomit::Vomit(double xc, double yc, int ddir, StudentWorld* w)
: Prop(IID_VOMIT, xc, yc, ddir, w), ttl(2){}

int Vomit::doSomething(){
    if(!isAlive())
        return GWSTATUS_CONTINUE_GAME;
    if(ttl==0){
        setDead();
        return GWSTATUS_CONTINUE_GAME;
    }
    hell()->poison(this);
    ttl--;
    return GWSTATUS_CONTINUE_GAME;
}

//-----------------------------------------------------------------------------------------

//Goodie Classes

VaccineGoodie::VaccineGoodie(double xc, double yc, StudentWorld* w)
: Goodie(IID_VACCINE_GOODIE, xc, yc, w){}

int VaccineGoodie::doSomething(){
    if(!isAlive())
        return GWSTATUS_CONTINUE_GAME;
    if(hell()->penelopeStepOn(this)){
        hell()->increaseScore(50);
        setDead();
        hell()->playSound(SOUND_GOT_GOODIE);
        hell()->pGetVaccine();
    }
    return GWSTATUS_CONTINUE_GAME;
}

GasCanGoodie::GasCanGoodie(double xc, double yc, StudentWorld* w)
: Goodie(IID_GAS_CAN_GOODIE, xc, yc, w){}

int GasCanGoodie::doSomething(){
    if(!isAlive())
        return GWSTATUS_CONTINUE_GAME;
    if(hell()->penelopeStepOn(this)){
        hell()->increaseScore(50);
        setDead();
        hell()->playSound(SOUND_GOT_GOODIE);
        hell()->pGetFlame();
    }
    return GWSTATUS_CONTINUE_GAME;
}

LandmineGoodie::LandmineGoodie(double xc, double yc, StudentWorld* w)
: Goodie(IID_LANDMINE_GOODIE, xc, yc, w){}

int LandmineGoodie::doSomething(){
    if(!isAlive())
        return GWSTATUS_CONTINUE_GAME;
    if(hell()->penelopeStepOn(this)){
        hell()->increaseScore(50);
        setDead();
        hell()->playSound(SOUND_GOT_GOODIE);
        hell()->pGetLandmine();
    }
    return GWSTATUS_CONTINUE_GAME;
}

//-----------------------------------------------------------------------------------------

//Good Peoples

Penelope::Penelope(double xc, double yc, StudentWorld* w)
: GoodPeople(IID_PLAYER, xc, yc, w){
    numLandmines = 0;
    numFlames = 0;
    numVaccines = 0;
}

void Penelope::incLandmines(){
    numLandmines+=2;
}

void Penelope::incFlames(){
    numFlames+=5;
}

void Penelope::incVaccines(){
    numVaccines++;
}

bool Penelope::useLandmine(){
    if(numLandmines>0){
        Actor* l = new Landmine(getX(), getY(), hell());
        hell()->addActor(l);
        numLandmines--;
        return true;
    }
    return false;
}

bool Penelope::useFlame(){
    if(getNumFlames()<=0)
        return false;
    numFlames--;
    hell()->playSound(SOUND_PLAYER_FIRE);
    int direction = getDirection();
    double x = getX();
    double y = getY();
    for(int i=0;i<3;i++){
        if(direction==up||direction==down){
            newCoord(x, y, direction, SPRITE_HEIGHT);
        } else {
            newCoord(x, y, direction, SPRITE_WIDTH);
        }
        if(!hell()->canFireTo(x, y))
            return false;
        Actor* f = new Flame(x,y,direction,hell());
        hell()->addActor(f);
    }
    return true;
}

bool Penelope::useVaccine(){
    if(numVaccines>0){
        cure();
        numVaccines--;
        return true;
    }
    return false;
}

int Penelope::getNumLandmines(){
    return numLandmines;
}

int Penelope::getNumFlames(){
    return numFlames;
}

int Penelope::getNumVaccines(){
    return numVaccines;
}

int Penelope::doSomething(){
    if(!isAlive())
        return GWSTATUS_PLAYER_DIED;
    
    if(infectToDeath()){
        setDead();
        hell()->playSound(SOUND_PLAYER_DIE);
        return GWSTATUS_PLAYER_DIED;
    }
    
    int key;
    if(!hell()->getKey(key))
        return GWSTATUS_CONTINUE_GAME;
    double dest_x = getX();
    double dest_y = getY();
    switch(key){
        case KEY_PRESS_RIGHT:
            setDirection(right);
            newCoord(dest_x, dest_y, right, 4);
            if(hell()->canMoveTo(dest_x, dest_y, this))
                moveTo(dest_x, dest_y);
            break;
        case KEY_PRESS_LEFT:
            setDirection(left);
            newCoord(dest_x, dest_y, left, 4);
            if(hell()->canMoveTo(dest_x, dest_y, this))
                moveTo(dest_x, dest_y);
            break;
        case KEY_PRESS_UP:
            setDirection(up);
            newCoord(dest_x, dest_y, up, 4);
            if(hell()->canMoveTo(dest_x, dest_y, this))
                moveTo(dest_x, dest_y);
            break;
        case KEY_PRESS_DOWN:
            setDirection(down);
            newCoord(dest_x, dest_y, down, 4);
            if(hell()->canMoveTo(dest_x, dest_y, this))
                moveTo(dest_x, dest_y);
            break;
        case KEY_PRESS_SPACE:
            if(getNumFlames()<=0)
                break;
            else
                useFlame();
            break;
        case KEY_PRESS_TAB:
            useLandmine();
            break;
        case KEY_PRESS_ENTER:
            useVaccine();
            break;
    }
    return GWSTATUS_CONTINUE_GAME;
}

Citizen::Citizen(double xc, double yc, StudentWorld* w)
: GoodPeople(IID_CITIZEN, xc, yc, w), paralyzed(false){}

int Citizen::doSomething(){
    if(!isAlive())
        return GWSTATUS_CONTINUE_GAME;
    if(infectToDeath()){
        setDead();
        hell()->playSound(SOUND_ZOMBIE_BORN);
        hell()->increaseScore(-1000);
        hell()->decreaseCitizen();
        int n = randInt(1,10);
        CanBeDamaged* z;
        switch(n){
            case 1:
            case 2:
            case 3:
                z = new SmartZombie(getX(), getY(), hell());
                break;
            default:
                z = new DumbZombie(getX(), getY(), hell());
        }
        hell()->addActor(z);
        return GWSTATUS_CONTINUE_GAME;
    }
    if(isParalyzed()){
        changeParalyzeStatus();
        return GWSTATUS_CONTINUE_GAME;
    }
    
    //Determine movement
    double dest_x = getX();
    double dest_y = getY();
    double dist_p = hell()->distp(this);
    double dist_z = hell()->distz(dest_x, dest_y);
    
    if((dist_p<dist_z||dist_z==-1)&&dist_p<=80){
        double targetX;
        double targetY;
        hell()->penelopeCoord(targetX, targetY);
        int direction2 = -1;
        int direction1 = approach(targetX, targetY, direction2);
        
        newCoord(dest_x, dest_y, direction1, 2);
        if(hell()->canMoveTo(dest_x, dest_y, this)){
            setDirection(direction1);
            moveTo(dest_x, dest_y);
            changeParalyzeStatus();
            return GWSTATUS_CONTINUE_GAME;
        }
        
        if(direction2!=-1){
            dest_x = getX();
            dest_y = getY();
            newCoord(dest_x, dest_y, direction2, 2);
            if(hell()->canMoveTo(dest_x, dest_y, this)){
                setDirection(direction2);
                moveTo(dest_x, dest_y);
                changeParalyzeStatus();
                return GWSTATUS_CONTINUE_GAME;
            }
        }
    }
    
    if(dist_z<=80){
        int maxFleeDir = -1;
        double maxDis = -1;
        double currentDis = -1;
        dest_x = getX();
        dest_y = getY();
        
        //check up
        if(hell()->canMoveTo(dest_x, dest_y+2, this)){
            currentDis = hell()->distz(dest_x, dest_y+2);
            if(currentDis>dist_z){
                maxFleeDir = up;
                maxDis = currentDis;
            }
        }
        //check down
        if(hell()->canMoveTo(dest_x, dest_y-2, this)){
            currentDis = hell()->distz(dest_x, dest_y-2);
            if(currentDis>dist_z&&currentDis>maxDis){
                maxFleeDir = down;
                maxDis = currentDis;
            }
        }
        //check left
        if(hell()->canMoveTo(dest_x-2, dest_y, this)){
            currentDis = hell()->distz(dest_x-2, dest_y);
            if(currentDis>dist_z&&currentDis>maxDis){
                maxFleeDir = left;
                maxDis = currentDis;
            }
        }
        //check right
        if(hell()->canMoveTo(dest_x+2, dest_y, this)){
            currentDis = hell()->distz(dest_x+2, dest_y);
            if(currentDis>dist_z&&currentDis>maxDis){
                maxFleeDir = right;
                maxDis = currentDis;
            }
        }
        
        if(maxFleeDir!=-1){
            setDirection(maxFleeDir);
            newCoord(dest_x, dest_y, maxFleeDir, 2);
            moveTo(dest_x, dest_y);
        }
    }
    
    changeParalyzeStatus();
    return GWSTATUS_CONTINUE_GAME;
}

void Citizen::damage(){
    hell()->playSound(SOUND_CITIZEN_DIE);
    setDead();
    hell()->decreaseCitizen();
    hell()->increaseScore(-1000);
}

void Citizen::infect(){
    if(isInfected())
        return;
    GoodPeople::infect();
    hell()->playSound(SOUND_CITIZEN_INFECTED);
}

//-----------------------------------------------------------------------------------------

DumbZombie::DumbZombie(double xc, double yc, StudentWorld* w)
: BadPeople(IID_ZOMBIE, xc, yc, w){}

int DumbZombie::doSomething(){
    if(!isAlive())
        return GWSTATUS_CONTINUE_GAME;
    if(isParalyzed()){
        changeParalyzeStatus();
        return GWSTATUS_CONTINUE_GAME;
    }
    
    //Determine vomit
    if(vomit()){
        changeParalyzeStatus();
        return GWSTATUS_CONTINUE_GAME;
    }
    
    //Determine move
    if(getMovementPlan()==0){
        planMovement();
        setRandomDirection();
    }
    zombieMove();
    changeParalyzeStatus();
    return GWSTATUS_CONTINUE_GAME;
}

void DumbZombie::damage(){
    BadPeople::damage();
    hell()->increaseScore(-1000);
    int i = randInt(1,10);
    if(i==1){
        int direction = getDirection();
        double x = getX();
        double y = getY();
        switch(direction){
            case up:
                y+=SPRITE_HEIGHT;
                break;
            case down:
                y-=SPRITE_HEIGHT;
                break;
            case left:
                x-=SPRITE_WIDTH;
            case right:
                x+=SPRITE_WIDTH;
        }
        VaccineGoodie* v = new VaccineGoodie(x,y,hell());
        if(!hell()->canCreateVaccine(v)){
            delete v;
            return;
        }
        hell()->addActor(v);
    }
}

SmartZombie::SmartZombie(double xc, double yc, StudentWorld* w)
: BadPeople(IID_ZOMBIE, xc, yc, w){}

int SmartZombie::doSomething(){
    if(!isAlive())
        return GWSTATUS_CONTINUE_GAME;
    if(isParalyzed()){
        changeParalyzeStatus();
        return GWSTATUS_CONTINUE_GAME;
    }
    
    //Determine vomit
    if(vomit()){
        changeParalyzeStatus();
        return GWSTATUS_CONTINUE_GAME;
    }
    
    //Determine move
    if(getMovementPlan()==0){
        planMovement();
    
        double targetX, targetY;
        int another = -1;
    
        if(hell()->smartScan(this, targetX, targetY)){
            setDirection(approach(targetX, targetY, another));
        } else {
            setRandomDirection();
        }
    }
    zombieMove();
    changeParalyzeStatus();
    return GWSTATUS_CONTINUE_GAME;
}

//-------------------------------------------------------------------------------------------

Landmine::Landmine(double xc, double yc, StudentWorld* w)
: CanBeDamaged(IID_LANDMINE, xc, yc, 0, 1, w), safetyTick(30), isActive(false){}

int Landmine::doSomething(){
    if(!isAlive())
        return GWSTATUS_CONTINUE_GAME;
    if(!isActive){
        safetyTick--;
        if(safetyTick==0)
            isActive = true;
        return GWSTATUS_CONTINUE_GAME;
    }
    if(hell()->triggerLandmine(this))
        trigger();
    return GWSTATUS_CONTINUE_GAME;
}

void Landmine::damage(){
    trigger();
}

void Landmine::trigger(){
    if(!isAlive())
        return;
    setDead();
    hell()->playSound(SOUND_LANDMINE_EXPLODE);
    double x = getX();
    double y = getY();
    Actor* a;// = new Flame(selfX, selfY, direction, hell());
    if(hell()->canFireTo(x, y)){
        a = new Flame(x, y, up, hell());
        hell()->addActor(a);
    }
    if(hell()->canFireTo(x+SPRITE_WIDTH, y)){
        a = new Flame(x+SPRITE_WIDTH, y, up, hell());
        hell()->addActor(a);
    }
    if(hell()->canFireTo(x-SPRITE_WIDTH, y)){
        a = new Flame(x-SPRITE_WIDTH, y, up, hell());
        hell()->addActor(a);
    }
    if(hell()->canFireTo(x, y+SPRITE_WIDTH)){
        a = new Flame(x, y+SPRITE_WIDTH, up, hell());
        hell()->addActor(a);
    }
    if(hell()->canFireTo(x, y-SPRITE_WIDTH)){
        a = new Flame(x, y-SPRITE_WIDTH, up, hell());
        hell()->addActor(a);
    }
    if(hell()->canFireTo(x+SPRITE_WIDTH, y+SPRITE_WIDTH)){
        a = new Flame(x+SPRITE_WIDTH, y+SPRITE_WIDTH, up, hell());
        hell()->addActor(a);
    }
    if(hell()->canFireTo(x-SPRITE_WIDTH, y+SPRITE_WIDTH)){
        a = new Flame(x-SPRITE_WIDTH, y+SPRITE_WIDTH, up, hell());
        hell()->addActor(a);
    }
    if(hell()->canFireTo(x+SPRITE_WIDTH, y-SPRITE_WIDTH)){
        a = new Flame(x+SPRITE_WIDTH, y-SPRITE_WIDTH, up, hell());
        hell()->addActor(a);
    }
    if(hell()->canFireTo(x-SPRITE_WIDTH, y-SPRITE_WIDTH)){
        a = new Flame(x-SPRITE_WIDTH, y-SPRITE_WIDTH, up, hell());
        hell()->addActor(a);
    }
    Actor* pit = new Pit(x, y, hell());
    hell()->addActor(pit);
}
