#ifndef ACTIONREQUEST_H
#define ACTIONREQUEST_H

enum class ActionRequest
{
    MoveForward,
    MoveBackward,
    RotateLeft90,
    RotateRight90,
    RotateLeft45,
    RotateRight45,
    Shoot,
    GetBattleInfo,
    DoNothing
};

#endif // ACTIONREQUEST_H
