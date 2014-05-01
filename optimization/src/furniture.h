#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <string>

class Furniture {
  public:
    Furniture(int id, std::string type, int angle, float  x, float  y){
      _id = id;
      _type = type;
      _angle = angle;
      _pos = glm::vec2(x,y);
    }
    
    Furniture(){
      _id = -1;
      _type = "none";
      _angle = 0;
      _pos = glm::vec2(-1,-1);
    }

    // Getter functions
    int  getID() const { return _id; }
    std::string  getType() const { return _type; }
    double getAngle() const { return _angle; }
    glm::vec2 getPos() const { return _pos; }

    // Setter functions
    void setID(int num){ _id = num; }
    void setType(std::string& t){ _type = t;}
    void setAngle(double a) { _angle = a;}
    void setPos(float x, float y){
      _pos = glm::vec2(x,y);
    }

    // utils
    void print(){
      std::cout << "item of type " << _type << " with  pos: (" << _pos.x << "," << _pos.y << ") with radian: " << _angle << std::endl; 
    }

    
  private:
    int _id;
    std::string _type;
    double _angle;
    glm::vec2 _pos;
};
