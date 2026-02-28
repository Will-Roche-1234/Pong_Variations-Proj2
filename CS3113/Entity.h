#ifndef ENTITY_H
#define ENTITY_H

#include "cs3113.h"

enum Direction { LEFT, UP, RIGHT, DOWN };

class Entity
{
private:
    Vector2 mPosition;
    Vector2 mMovement;

    Vector2 mScale;
    Vector2 mColliderDimensions;

    Texture2D mCurrentTexture;
    TextureType mTextureType;
    Vector2 mSpriteSheetDimensions;

    std::vector<int> mAnimationIndices;
    Direction mDirection;
    int mFrameSpeed;

    int mCurrentFrameIndex = 0;
    float mAnimationTime = 0.0f;

    float mSpeed;
    float mAngle;

    bool isColliding(Entity *other) const;
    void animate(float deltaTime);

public:
    static const int   DEFAULT_SIZE        = 250;
    static constexpr float DEFAULT_SPEED  = 200.0f;
    static const int   DEFAULT_FRAME_SPEED = 8;

    Entity(Vector2 position, Vector2 scale, const char *textureFilepath);
    ~Entity();

    void update(float deltaTime);
    void render();
    void normaliseMovement() { Normalise(&mMovement); };

    void moveUp()    { mMovement.y = -1; mDirection = UP;    }
    void moveDown()  { mMovement.y =  1; mDirection = DOWN;  }

    void resetMovement() { mMovement = { 0.0f, 0.0f }; }

    bool checkCollision(Entity *other) const { return isColliding(other); }

    Vector2     getPosition()           const { return mPosition;           }
    Vector2     getMovement()           const { return mMovement;           }
    Vector2     getScale()              const { return mScale;              }
    Vector2     getColliderDimensions() const { return mScale;              }
    TextureType getTextureType()        const { return mTextureType;        }
    int         getFrameSpeed()         const { return mFrameSpeed;         }
    float       getSpeed()              const { return mSpeed;              }
    float       getAngle()              const { return mAngle;              }

    void setPosition(Vector2 newPosition)
        { mPosition = newPosition;             }
    void setMovement(Vector2 newMovement)
        { mMovement = newMovement;             }
    void setScale(Vector2 newScale)
        { mScale = newScale;                   }
    void setColliderDimensions(Vector2 newDimensions)
        { mColliderDimensions = newDimensions; }
    void setSpeed(float newSpeed)
        { mSpeed = newSpeed;                   }
    void setFrameSpeed(int newSpeed)
        { mFrameSpeed = newSpeed;              }
    void setAngle(float newAngle)
        { mAngle = newAngle;                   }
};

#endif // ENTITY_H
