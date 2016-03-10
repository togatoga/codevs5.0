#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>
#include <set>
#include <cassert>
using namespace std;

const int INF = 876765346;

const char CELL_EMPTY = '_';
const char CELL_WALL ='W';
const char CELL_OBJECT = 'O';

class Point {
public:
  int x, y;
  Point() { x = y = -1; }
  Point(int x, int y): x(x), y(y) {}
  bool operator== (const Point &p) const { return x == p.x && y == p.y; }
  bool operator < (const Point &p) const {
    if (x == p.x){
      return y < p.y;
    }
    return x < p.x;
  }
};

class Skill {
public:
  int id, cost;
  Skill() { id = cost = -1; }
  Skill(int id, int cost): id(id), cost(cost) {}

  static Skill input(int id) {
    int cost;
    cin >> cost;
    return Skill(id, cost);
  }
};

class Cell : public Point {
public:
  char kind;
  bool containsNinja, containsSoul, containsDog;
  Cell() {
    kind = '?';
    containsNinja = containsSoul = containsDog = false;
  }
  Cell(int x, int y, char kind): Point(x, y), kind(kind) {
    containsNinja = containsSoul = containsDog = false;
  }

  bool isWall() const { return kind == CELL_WALL; }
  bool isObject() const { return kind == CELL_OBJECT; }
  bool isEmpty() const { return kind == CELL_EMPTY; }
  //togasaki
  bool isNoCharacter() const {
    return !containsSoul & !containsNinja & !containsDog;
  }
};

class Character : public Point {
public:
  int id;
  Character() { id = -1; }
  Character(int id, int x, int y): id(id), Point(x, y) {}

  static Character input() {
    int id, x, y;
    cin >> id >> y >> x;
    return Character(id, x, y);
  }
};

class State {
public:
  int skillPoint;
  int H, W;
  vector< vector<Cell> > field;
  vector<Character> ninjas;
  vector<Character> dogs;
  vector<Point> souls;
  vector<int> skillCount;
  //togasaki
  double searchValue;
  int getSoul;
  //  bool fail;
  int commandId;
  vector<int> minDistSoulById;
  int rivalSkillPoint;
  
  int skillUseId;
  int skillId;
  int skillDepth;
  Point targetPoint;
  int nextRivalAttack;
  
  int skillRivalId;
  int skillRivalDepth;

  Point targetRivalPoint;
  vector<int> survive;

  int reachDeath;
  int hammingDistance;
  int stepNum;
  vector<Point> shadowNinjas;
  State() {
    skillPoint = H = W = -1;
    field.clear();
    ninjas.clear();
    dogs.clear();
    souls.clear();
    skillCount.clear();
    //togasaki
    searchValue = 0;
    getSoul = 0;
    //    fail = false;
    commandId = -1;
    minDistSoulById.clear();
    rivalSkillPoint = 0;
    skillUseId = -1;
    skillId = -1;
    skillDepth = -1;
    targetPoint = Point(-1,-1);
    nextRivalAttack = INF;
    
    skillRivalId = -1;
    skillRivalDepth = -1;
    targetRivalPoint = Point(-1,-1);
    survive.clear();

    reachDeath = 0;
    hammingDistance = 0;
    stepNum = 0;
  }

  static State input(int numOfSkills) {
    State st;

    cin >> st.skillPoint;
    cin >> st.H >> st.W;

    st.field.clear();

    for (int i = 0; i < st.H; i++) {
      vector<Cell> line;
      string s;
      cin >> s;
      for (int j = 0; j < s.size(); j++) {
        line.push_back(Cell(j, i, s[j]));
      }
      st.field.push_back(line);
    }

    int numOfNinjas;
    cin >> numOfNinjas;
    st.ninjas.clear();
    st.minDistSoulById.clear();
    for (int i = 0; i < numOfNinjas; i++) {
      Character ninja = Character::input();
      st.ninjas.push_back(ninja);
      st.minDistSoulById.push_back(INF);
      st.field[ninja.y][ninja.x].containsNinja = true;
    }

    int numOfDogs;
    cin >> numOfDogs;
    st.dogs.clear();
    for (int i = 0; i < numOfDogs; i++) {
      Character dog = Character::input();
      st.dogs.push_back(dog);
      st.field[dog.y][dog.x].containsDog = true;
    }

    int numOfSouls;
    cin >> numOfSouls;
    st.souls = vector<Point>();
    for (int i = 0; i < numOfSouls; i++) {
      int x, y;
      cin >> y >> x;
      st.souls.push_back(Point(x, y));
      st.field[y][x].containsSoul = true;
    }

    st.skillCount = vector<int>();
    for (int i = 0; i < numOfSkills; i++) {
      int count;
      cin >> count;
      st.skillCount.push_back(count);
    }

    return st;
  }

  bool operator < (const State &right) const {
    
    if (survive[0] == -1 && right.survive[0] == -1){//probably death
      if (skillId != -1 && right.skillId == -1){
	return false;
      }
      if (skillId == -1 && skillId != -1){
	return true;
      }
    }
    
    for (int i = 0; i < survive.size(); i++){
      if (survive[i] < right.survive[i]){
    	return true;
      }
      if (survive[i] > right.survive[i]){
    	return false;
      }

    }
    
    if (getSoul < right.getSoul){
      return true;
    }
    if (getSoul > right.getSoul){
      return false;
    }
    

    if(minDistSoulById[0] + minDistSoulById[1] ==  right.minDistSoulById[0] + right.minDistSoulById[1]){

      if (hammingDistance < right.hammingDistance){
	return true;
      }
      if (hammingDistance > right.hammingDistance){
	return false;
      }
      if (stepNum < right.stepNum){
	return true;
      }
      if (stepNum > right.stepNum){
	return false;
      }

      return skillPoint < right.skillPoint;
    }
    return minDistSoulById[0] + minDistSoulById[1] > right.minDistSoulById[0] + right.minDistSoulById[1];
  }
};

class Search : public Point {
public:
  int dist;
  Search(){}
  Search(int x, int y, int dist): Point(x, y), dist(dist) {}
};

class Attack{
public:
  int skillId;
  Point targetPoint;
  Attack(){
    skillId = -1;
    targetPoint = Point(-1, -1);
  }
  Attack(int id, int x, int y):skillId(id),targetPoint(x, y){}
  Attack(int id, Point targetPoint):skillId(id),targetPoint(targetPoint){}
  void setSkill(int id, Point target){
    skillId = id;
    targetPoint = target;
  }
  
};
vector<Skill> skills;
class Order{
public:
  int comId;
  int skillUseId;
  int skillId;
  int skillCost;
  Point targetPoint;
  Order(){
    comId = -1;
    skillId = -1;
    targetPoint = Point(-1, -1);
    skillCost = 0;
    skillUseId = -1;
  }
  Order(int comId, int skillUseId,int skillId, int skillCost,int x, int y):comId(comId),skillUseId(skillUseId),skillId(skillId),skillCost(skillCost),targetPoint(x, y){}

  void setOrder(int id){
    comId = id;
  }
  void setSkill(int useId, int id){
    skillUseId = useId;
    skillId = id;
    skillCost = skills[id].cost;
  }
  void setSkill(int id){
    skillId = id;
    skillCost = skills[id].cost;
  }
  void setTargetPoint(int x, int y){
    targetPoint = Point(x, y);
  }
};


int remTime;
//vector<Skill> skills;
State myState;
State rivalState;

int dx[] =    {  0,   1,   0,  -1,   0};
int dy[] =    { -1,   0,   1,   0,   0};
string ds[] = {"U", "R", "D", "L",  ""};
//togasaki
int dogDx[] =    {  0,   -1,   1,  0,   0};//U L R D
int dogDy[] =    { -1,   0,   0,   1,   0};
vector<vector<string> > commands;
void useShadowClone(const State& nowState, const Order &order, vector<Order> &result){
  if (nowState.skillPoint < skills[5].cost){
    return ;
  }
  Order next = order;
  for (int id = 0; id < 2; id++){
    int px = nowState.ninjas[id].x;
    int py = nowState.ninjas[id].y;

    //upper left;
    for (int y = py - 1; y >= 1; y--){
      for (int x = px - 1; x >= 1; x--){
	if (nowState.field[y][x].isWall() || nowState.field[y][x].isObject())continue;
	next.setSkill(5);
	next.setTargetPoint(x, y);
	result.push_back(next);
	goto NextSegment1;
      }
    }
  NextSegment1:;
    //upper right
    for (int y = py - 1; y >= 1; y--){
      for (int x = px + 1; x < nowState.H - 1; x++){
	if (nowState.field[y][x].isWall() || nowState.field[y][x].isObject())continue;
	next.setSkill(5);
	next.setTargetPoint(x, y);
	result.push_back(next);
	goto NextSegment2;
      }
    }
  NextSegment2:;
    
    //lower left
    for (int y = py + 1; y < nowState.H - 1; y++){
      for (int x = px - 1; x >= 1; x--){
	if (nowState.field[y][x].isWall() || nowState.field[y][x].isObject())continue;
next.setSkill(5);
	next.setTargetPoint(x, y);
	result.push_back(next);
	goto NextSegment3;
      }
    }
  NextSegment3:;
    //lower right
    for (int y = py + 1; y < nowState.H - 1; y++){
      for (int x = px + 1; x < nowState.W - 1; x++){
	if (nowState.field[y][x].isWall() || nowState.field[y][x].isObject())continue;
	next.setSkill(5);
	next.setTargetPoint(x, y);
	result.push_back(next);
	goto NextSegment4;
      }
    }
  NextSegment4:;
  }
  return ;
}
void useLightning(const State& nowState, const Order &order, vector<Order> &result){
  if (nowState.skillPoint < skills[3].cost){
    return ;
  }
  Order next = order;
  for (int id = 0; id < 2; id++){
    int px = nowState.ninjas[id].x;
    int py = nowState.ninjas[id].y;
    for (int y = -2; y <= 2; y++){
      for (int x = -2; x <= 2; x++){
	int lightX = px + x;
	int lightY = py + y;
	if (lightX <= 0 || lightX >= nowState.W - 1 || lightY <= 0 || lightY >= nowState.H - 1){
	  continue;
	}
	if (!nowState.field[lightY][lightX].isObject())continue;
	next.setSkill(3);
	next.setTargetPoint(lightX, lightY);
	result.push_back(next);
      }
    }
  }
  return ;
}

void attackFallRock(const State& myState, const State& rivalState, vector<Attack> &result){
  if (rivalState.skillPoint < skills[2].cost){
    return ;
  }
  for (int id = 0; id < 2; id++){
    int px = myState.ninjas[id].x;
    int py = myState.ninjas[id].y;
    for (int y = -2; y <= 2; y++){
      for (int x = -2; x <= 2; x++){
	int fallx = px + x;
	int fally = py + y;
	if (abs(x) + abs(y) > 3)continue;
	if (fallx <= 0 || fallx >= myState.W - 1 || fally <= 0 || fally >= myState.H - 1){
	  continue;
	}
	if (!myState.field[fally][fallx].isEmpty() || !myState.field[fally][fallx].isNoCharacter()){
	  continue;
	}
	Attack attack;
	assert(fallx >= 0 && fally >= 0);
	attack.setSkill(2, Point(fallx, fally));
	result.push_back(attack);
      }
    }
  }
  return ;
}
vector<Attack> possibleAttack(const State& myState, const State& rivalState){
  vector<Attack> result;
  result.push_back(Attack());//None
  attackFallRock(myState, rivalState, result);
  
  return result;
}
bool validateOrder(const State& nowState, int comId, int skillId){

  
  for (int id = 0; id < 2; id++){
    int px = nowState.ninjas[id].x;
    int py = nowState.ninjas[id].y;
    int nx = px;
    int ny = py;
    for (int i = 0; i < commands[comId][id].size(); i++){
      nx += dx[commands[comId][id][i] - '0'];
      ny += dy[commands[comId][id][i] - '0'];
      if (nowState.field[ny][nx].isWall()){
	return false;
      }
      if (nowState.field[ny][nx].isObject()){
	int nnx = nx + dx[commands[comId][id][i] - '0'];	
	int nny = ny + dy[commands[comId][id][i] - '0'];
	if (!nowState.field[nny][nnx].isEmpty() || nowState.field[nny][nnx].containsDog || nowState.field[nny][nnx].containsNinja){
	  if (skillId == -1){
	    return false;
	  }
	}
      }
    }
  }
  return true;
}

void useWhirlslash(const State& nowState, int id, const Order &order, vector<Order> &result){
  if (nowState.skillPoint < skills[7].cost){
    return ;
  }
  int px = nowState.ninjas[id].x;
  int py = nowState.ninjas[id].y;
  Order nextOrder = order;
  for (int y = -1; y <= 1; y++){
    for (int x = -1; x <= 1; x++){
      if (nowState.field[py + y][px + x].containsDog){
	nextOrder.setSkill(id, 7);
	result.push_back(nextOrder);
	return ;
      }
    }
  }
  return ;
}
unsigned long xor128(void){
  static unsigned long x=123456789,y=362436069,z=521288629,w=88675123;
  unsigned long t;
  t=(x^(x<<11));x=y;y=z;z=w; return( w=(w^(w>>19))^(t^(t>>8)) );
}

vector<Order> possibleOrder(const State& nowState, int depth, bool useSpecialSkill){
  vector<Order> result;
  Order nowOrder;


    for (int i = 0; i < commands.size(); i++){
      nowOrder.setOrder(i);
      if (validateOrder(nowState, i, -1)){
	result.push_back(nowOrder);
      }

      if (depth == 0 || depth == 1){
	if (!useSpecialSkill){
	  //2

	  //5
	  //useShadowClone(nowState, nowOrder, result);
	}
	//7
	if (useSpecialSkill){
	  useLightning(nowState, nowOrder, result);	  	  //2
	  //	  useLightning(nowState, nowOrder, result);
	  for (int id = 0; id < 2; id++){
	    useWhirlslash(nowState, id, nowOrder,result);
	  }
	}
      }
    }

  return result;
}


vector<vector<string> > createCommands(){
  vector<vector<string> > result;
  vector<string> tmp;
  for (int i = 0; i < 5; i++){
    for (int j = 0; j < 5; j++){
      string com = to_string(i) + to_string(j);
      tmp.push_back(com);
    }
  }

  for (int i = 0; i < tmp.size(); i++){
    for (int j = 0; j < tmp.size(); j++){
      vector<string> res;
      res.push_back(tmp[i]);
      res.push_back(tmp[j]);
      result.push_back(res);
    }
  }

  return result;
}

void calculateMinDistToSoul(State &nowState){

  for (int id = 0; id < 2; id++){
    int sx = nowState.ninjas[id].x;
    int sy = nowState.ninjas[id].y;
    queue<Search> open;
    vector< vector<bool> > closed(nowState.H, vector<bool>(nowState.W, false));

    vector< vector<Cell> > field = nowState.field;
    closed[sy][sx] = true;
    open.push(Search(sx, sy, 0));
    while (!open.empty()){
      Search sc = open.front();
      open.pop();
      if (!field[sy][sx].isEmpty())continue;
      for (int dir = 0; dir < 4; dir++){
	int nx = sc.x + dx[dir];
	int ny = sc.y + dy[dir];
	if (field[ny][nx].isWall())continue;
	if (closed[ny][nx])continue;
	if (field[ny][nx].containsDog)continue;
	if (field[ny][nx].isObject()){
	  int nnx = nx + dx[dir];
	  int nny = ny + dy[dir];
	  if (!field[nny][nnx].isEmpty())continue;
	  swap(field[nny][nnx].kind, field[ny][nx].kind);
	}
	if (field[ny][nx].containsSoul){
	  if (field[ny][nx].isObject()){
	    int cnt = 0;
	    for (int i = 0; i < 4; i++){
	      int nnx = nx + dx[dir];
	      int nny = ny + dy[dir];
	      if (!field[nny][nnx].isWall()){
		cnt++;
	      }
	    }
	    if (cnt == 2)continue;
	  }
	  nowState.minDistSoulById[id] = sc.dist + 1;
	  goto NextId;
	}
	closed[ny][nx] = true;
	open.push(Search(nx, ny, sc.dist + 1));
      }
    }
  NextId:;
  }
  return ;

}

void checkReachDeath(State& nowState){
  for (int id = 0; id < 2; id++){
    int x = nowState.ninjas[id].x;
    int y = nowState.ninjas[id].y;
    int wall = 0;
    int rock = 0;
    int dog = 0;
    for (int k = 0; k < 4; k++){
      int nx = x + dx[k];
      int ny = y + dy[k];
      if (nowState.field[ny][nx].containsDog){
	dog++;
      }
      if (nowState.field[ny][nx].isObject()){
	rock++;
      }
      if (nowState.field[ny][nx].isWall()){
	wall++;
      }
    }
    if (dog > 0){
      nowState.reachDeath = dog + rock;
    }
  }

}
void simulateNextDog(State &nowState, int depth){
  if (false && depth == 0 && nowState.shadowNinjas.size() > 0){
    //    cerr << "UNKO" << endl;
    vector<vector<int> > dist(nowState.H, vector<int>(nowState.W, INF));
    //    cerr << nowState,shadowNinjas.size() << endl;
    for (int id = 0; id < nowState.shadowNinjas.size(); id++){
      int sx = nowState.shadowNinjas[id].x;
      int sy = nowState.shadowNinjas[id].y;

      queue<Search> open;
      vector< vector<bool> > closed(nowState.H, vector<bool>(nowState.W, false));

      closed[sy][sx] = true;
      open.push(Search(sx, sy, 0));
      while (!open.empty()) {
	Search sc = open.front(); open.pop();
	//	cout << sc.y << " " << sc.x << endl;
	dist[sc.y][sc.x] = sc.dist;
	//	cerr << "cnt = " << cnt << endl;
	for (int dir = 0; dir < 4; dir++) {
	  int nx = sc.x + dx[dir];
	  int ny = sc.y + dy[dir];
	  
	  if (!nowState.field[ny][nx].isEmpty()) continue;
	  if (closed[ny][nx]) continue;

	  closed[ny][nx] = true;
	  if (dist[ny][nx] > sc.dist + 1){
	    open.push(Search(nx, ny, sc.dist + 1));
	  }
	}
      }
    }
    //    cerr << "OrderDog" << endl;
    vector<pair<int, int> > orderDog;
    for (int i = 0; i < nowState.dogs.size(); i++){
      int px = nowState.dogs[i].x;
      int py = nowState.dogs[i].y;
      orderDog.push_back(make_pair(dist[py][px], i));
    }
    sort(orderDog.begin(), orderDog.end());

    for (int i = 0; i < orderDog.size(); i++){
      int id = orderDog[i].second;
      int px = nowState.dogs[id].x;
      int py = nowState.dogs[id].y;
      int nowDist = dist[py][px];
      for (int k = 0; k < 4; k++){
	int nx = px + dogDx[k];
	int ny = py + dogDy[k];
	int nextDist = dist[ny][nx];
	if (nowState.field[ny][nx].isEmpty() && !nowState.field[ny][nx].containsDog && (nowDist - nextDist) == 1){
	  nowState.field[py][px].containsDog = false;
	  nowState.field[ny][nx].containsDog = true;
	  nowState.dogs[id].x = nx;
	  nowState.dogs[id].y = ny;
	  break;
	}
      }
    }
    //    cerr << "Done!!!!" << endl;
  }else{
    vector<vector<int> > dist(nowState.H, vector<int>(nowState.W, INF));
    for (int id = 0; id < 2; id++){
      int sx = nowState.ninjas[id].x;
      int sy = nowState.ninjas[id].y;
      queue<Search> open;
      vector< vector<bool> > closed(nowState.H, vector<bool>(nowState.W, false));
      closed[sy][sx] = true;
      open.push(Search(sx, sy, 0));
      while (!open.empty()) {
	Search sc = open.front(); open.pop();
	dist[sc.y][sc.x] = sc.dist;
	
	for (int dir = 0; dir < 4; dir++) {
	  int nx = sc.x + dx[dir];
	  int ny = sc.y + dy[dir];

	  if (!nowState.field[ny][nx].isEmpty()) continue;
	  if (closed[ny][nx]) continue;

	  closed[ny][nx] = true;
	  if (dist[ny][nx] > sc.dist + 1){
	    open.push(Search(nx, ny, sc.dist + 1));
	  }
	}
      }
    }
    vector<pair<int, int> > orderDog;
    for (int i = 0; i < nowState.dogs.size(); i++){
      int px = nowState.dogs[i].x;
      int py = nowState.dogs[i].y;
      orderDog.push_back(make_pair(dist[py][px], i));
    }
    sort(orderDog.begin(), orderDog.end());

    for (int i = 0; i < orderDog.size(); i++){
      int id = orderDog[i].second;
      int px = nowState.dogs[id].x;
      int py = nowState.dogs[id].y;
      int nowDist = dist[py][px];
      for (int k = 0; k < 4; k++){
	int nx = px + dogDx[k];
	int ny = py + dogDy[k];
	int nextDist = dist[ny][nx];
	if (nowState.field[ny][nx].isEmpty() && !nowState.field[ny][nx].containsDog && (nowDist - nextDist) == 1){
	  nowState.field[py][px].containsDog = false;
	  nowState.field[ny][nx].containsDog = true;
	  nowState.dogs[id].x = nx;
	  nowState.dogs[id].y = ny;
	  break;
	}
      }
    }
  }
  //  cerr << "Done!!!!" << endl;;
  return ;
}



//-1 killed 0 fail 1 success 
int genNextState(State &nextState, const vector<string> &command, bool shadow=false){

  for (int id = 0; id < 2; id++){
    string com = command[id];
    for (int i = 0; i < com.size(); i++){
      int px = nextState.ninjas[id].x;
      int py = nextState.ninjas[id].y;
      int nx = px + dx[com[i] - '0'];
      int ny = py + dy[com[i] - '0'];
      
      if (nextState.field[ny][nx].isWall()){
	return -1;
      }

      if (nextState.field[ny][nx].isObject()){//rock
	//next empty
	int nnx = nx + dx[com[i] - '0'];
	int nny = ny + dy[com[i] - '0'];
	if (nextState.field[nny][nnx].isWall() || !nextState.field[nny][nnx].isEmpty() || nextState.field[nny][nnx].containsDog || nextState.field[nny][nnx].containsNinja){

	  continue;
	}
	swap(nextState.field[ny][nx].kind, nextState.field[nny][nnx].kind);
      }

      if (nextState.field[ny][nx].containsSoul){//find soul
	nextState.skillPoint += 2;
	nextState.getSoul += 1;
	nextState.field[ny][nx].containsSoul = false;
	nextState.souls.erase( find(nextState.souls.begin(), nextState.souls.end(), Point(nx, ny)) );
      }
      //next

      nextState.field[py][px].containsNinja = false;
      nextState.field[ny][nx].containsNinja = true;
      nextState.ninjas[id].x = nx;
      nextState.ninjas[id].y = ny;
      if (com[i] != '4'){
	nextState.stepNum++;
      }
    }
    //next killed by dog
  }
  if (!shadow){
    for (int id = 0; id < 2; id++){
      for (int i = 0; i < 5; i++){
	int nx = nextState.ninjas[id].x + dx[i];
	int ny = nextState.ninjas[id].y + dy[i];
	if (nextState.field[ny][nx].containsDog){
	  return -1;//killed
	}
      }
    }
  }
  
  return 1;
}


void simulateAttack(State &nowState, const Attack &attack){
  int skillId = attack.skillId;
  int tarX = attack.targetPoint.x;
  int tarY = attack.targetPoint.y;
  if (skillId == 2){
    nowState.field[tarY][tarX].kind = 'O';
  }
  return;
}

void simulateDefence(State& nowState, int skillUseId,int skillId, Point targetPoint){
  int tarX = targetPoint.x;
  int tarY = targetPoint.y;
  if (skillId == -1){
    return ;
  }
  if (skillId == 3){
    if (nowState.field[tarY][tarX].isObject()){
      nowState.field[tarY][tarX].kind = '_';
      return ;
    }
  }
  if (skillId == 5){
    if (!nowState.field[tarY][tarX].isWall() && !nowState.field[tarY][tarX].isObject() && tarX != -1 && tarY != -1){
      nowState.shadowNinjas.push_back(Point(targetPoint));
    }
    return ;
  }
  if (skillId == 7){
    int px = nowState.ninjas[skillUseId].x;
    int py = nowState.ninjas[skillUseId].y;
    for (int y = -1; y <= 1; y++){
      for (int x = -1; x <= 1; x++){
	int nx = px + x;
	int ny = py + y;
	nowState.field[ny][nx].containsDog = false;
      }
    }
    return ;
  }
  return ;
}
void calculateHammingDistance(State& state){
  int px = state.ninjas[0].x;
  int py = state.ninjas[0].y;
  
  int qx = state.ninjas[1].x;
  int qy = state.ninjas[1].y;

  state.hammingDistance = abs(px - qx) + abs(py - qy);


  return ;
}
void showState(const vector<State> &currentState){
  cerr << "-----------------------" << endl;
  for (int i = 0; i < currentState.size(); i++){
    cout << currentState[i].getSoul << endl;
  }
  cerr << "-----------------------" << endl;
}
/*
 * このAIについて
 * - 各忍者について、 thinkByNinja(id) を2回行います。
 * - thinkByNinja(id) は、一人の忍者の一歩の行動を決める関数です(詳しくは関数のコメントを参照)。
 *
 * - 忍術
 * -- 「超高速」のみを使用します。
 * -- 「超高速」を使えるだけの忍力を所持している場合に自動的に使用して、thinkByNinja(id) を1回多く呼び出します。
 */
void think(int depthLimit, int beamWidth=5) {
  vector<State> currentState[depthLimit + 1];
  currentState[0].push_back(myState);
  //depth 0

  set<int> tabooCommands;
  int cntChallenge = 0;  
  for (int depth = 0; depth < depthLimit; depth++){

    if (currentState[depth].size() > beamWidth){
      sort(currentState[depth].rbegin(), currentState[depth].rend());
      currentState[depth].erase(currentState[depth].begin() + beamWidth, currentState[depth].end());
    }
    cntChallenge++;
    //    cerr << depth << " " << k << " " << cntChallenge << endl;      
    for (int k = 0; k < currentState[depth].size(); k++){


      vector<Order> myOrders = possibleOrder(currentState[depth][k], depth, cntChallenge >= 2);
      vector<Attack> rivalAttacks;
      if (depth < 2){
	rivalAttacks = possibleAttack(currentState[depth][k], rivalState);
      }else{
	rivalAttacks.push_back(Attack());
      }

      //      cerr <<currentState[depth].size() << " " << myOrders.size() << " " << rivalAttacks.size() << endl;
      for (int i = 0; i < myOrders.size(); i++){
	int survive = 1;
	int comId = myOrders[i].comId;
	int skillUseId = myOrders[i].skillUseId;
	int skillId = myOrders[i].skillId;
	int skillCost = myOrders[i].skillCost;
	Point targetPoint = myOrders[i].targetPoint;

	//rival
	Point targetRivalPoint = Point(-1, -1);
	int skillRivalId = -1;
	int skillRivalCost = skillRivalId >= 0 ? skills[skillRivalId].cost : 0;
	for (int j = 0; j < rivalAttacks.size(); j++){
	  State nextState = currentState[depth][k];
	  simulateAttack(nextState, rivalAttacks[j]);
	  simulateDefence(nextState, skillUseId, skillId, targetPoint);
	  int tmp = genNextState(nextState, commands[comId], skillId == 5);
	  if (tmp == -1){//killed
	    if (rivalAttacks[j].skillId != -1){//use skill
	      survive = -1;
	      skillRivalId = rivalAttacks[j].skillId;
	      targetRivalPoint = rivalAttacks[j].targetPoint;
	    }else{//unused skill
	      survive = -2;
	      break;
	    }
	  }

	  if (tmp == 1){//survive
	    if (skillId == 5){//use shadowClaone
	      simulateNextDog(nextState, depth);
	      for (int id = 0; id < 2; id++){
	    	int x = nextState.ninjas[id].x;
	    	int y = nextState.ninjas[id].y;
		if (nextState.field[y][x].containsDog){//death
	    	  if (rivalAttacks[j].skillId != -1){//use skillId
	    	    survive = -1;
		    skillRivalId = rivalAttacks[j].skillId;
		    targetRivalPoint = rivalAttacks[j].targetPoint;
	    	  }else{//unused skill
	    	    survive = -2;
	    	    break;
	    	  }
	    	}
	      }
	    }
	    if (survive == -2){
	      break;
	    }
	  }
	}
	
	if (survive == 1 || survive == -1 ||currentState[depth + 1].empty()){
	  State nextState = currentState[depth][k];
	  Attack nowAttack = Attack(skillRivalId, targetRivalPoint);
	  simulateAttack(nextState, nowAttack);
	  simulateDefence(nextState, skillUseId, skillId, targetPoint);
	  genNextState(nextState, commands[comId]);//survive
	  simulateNextDog(nextState, depth);
	  nextState.survive.push_back(survive);
	  if (depth == 0){
	    nextState.commandId = comId;
	    nextState.skillUseId = skillUseId;
	    nextState.skillId = skillId;
	    nextState.targetPoint = targetPoint;
	  }
	  nextState.skillPoint -= skillCost;
	  nextState.rivalSkillPoint -= skillRivalCost;
	  //	  cerr << skillRivalId << " " << targetRivalPoint.x << " " << targetRivalPoint.y << endl;
	  
	  //additional score
	  calculateMinDistToSoul(nextState);
	  calculateHammingDistance(nextState);
	  currentState[depth + 1].push_back(nextState);
	}
      }
    }
    
    //    currentState[depth].clear();
    if (cntChallenge == 1){
      sort(currentState[depth + 1].rbegin(), currentState[depth + 1].rend());
      if (currentState[depth + 1].empty() || currentState[depth + 1][0].survive[depth] != 1){//use special skill
	//currentState[depth + 1].clear();
	depth -= 1;
	continue;
      }
    }
    cerr << "Done!!!!" << endl;
    cntChallenge = 0;
  }

  for (int depth = depthLimit; depth >= 1; depth--){
    sort(currentState[depth].rbegin(), currentState[depth].rend());
    //    cerr << currentState[depth].size() << endl;
    for (int i = 0; i < currentState[depth].size(); i++){
      int comId = currentState[depth][i].commandId;
      int skillUseId = currentState[depth][i].skillUseId;
      int skillId = currentState[depth][i].skillId;
      int tarX = currentState[depth][i].targetPoint.x;
      int tarY = currentState[depth][i].targetPoint.y;
      
      //my
      int p1x = currentState[depth][i].ninjas[0].x;
      int p1y = currentState[depth][i].ninjas[0].y;
      int p2x = currentState[depth][i].ninjas[1].x;
      int p2y = currentState[depth][i].ninjas[1].y;

      // cerr << "comId = " << comId << " " << currentState[depth][i].survive[0] << endl;
      // cout << "p1x = " << p1x << " " << "p1y = " << p1y << endl;
      // cout << "p2x = " << p2x << " " << "p2y = " << p2y << endl;

      if (skillId != -1){//use skill
	if (skillId == 3){
	  cout << 3 << endl;
	  cout << 3 << " " << tarY << " " << tarX << endl;
	}
	if (skillId == 5){
	  cout << 3 << endl;
	  cout << 5 << " " << tarY << " " << tarX << endl;	  
	}
	if (skillId == 7){
	  cout << 3 << endl;
	  cout << 7 << " " << skillUseId << endl;
	}
	for (int j = 0; j < commands[comId].size(); j++){
	  string com = commands[comId][j];
	  for (int k = 0; k < com.size(); k++){
	    cout << ds[com[k] - '0'];
	  }
	  cout << endl;
	}
	return ;
      }else{//unused skill
	cout << 2 << endl;
	for (int j = 0; j < commands[comId].size(); j++){
	  string com = commands[comId][j];
	  for (int k = 0; k < com.size(); k++){
	    cout << ds[com[k] - '0'];
	  }
	  cout << endl;
	}
	return ;
      }
    }
  }
  cout << 2 << endl;
  cout << endl;
  cout << endl;
  return ;
}

bool input() {
  if (!(cin >> remTime)) return false;

  int numOfSkills;
  cin >> numOfSkills;

  skills.clear();
  for (int i = 0; i < numOfSkills; i++) {
    skills.push_back(Skill::input(i));
  }

  myState = State::input(skills.size());
  rivalState = State::input(skills.size());
  myState.rivalSkillPoint = rivalState.skillPoint;
  return true;
}

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(0);
  // AIの名前を出力
  cout << "TogaTogAI" << endl;
  cout.flush();
  commands = createCommands();
  while (input()) {
    think(3);
    cout.flush();
  }

  return 0;
}
