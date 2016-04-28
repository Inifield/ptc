/*
Written by Xinef
 */

#ifndef OREGON_ESCORTMOVEMENTGENERATOR_H
#define OREGON_ESCORTMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

template<class T>
class EscortMovementGenerator
    : public MovementGeneratorMedium< T, EscortMovementGenerator<T> >
{
    public:
        EscortMovementGenerator(Movement::PointsArray* _path = NULL) : i_recalculateSpeed(false)
        {
            if (_path)
                m_precomputedPath = *_path;
        }

        void Initialize(T&);
        void Finalize(T&);
        void Reset(T&);
        bool Update(T&, const uint32&);

        void unitSpeedChanged() { i_recalculateSpeed = true; }

        MovementGeneratorType GetMovementGeneratorType() { return ESCORT_MOTION_TYPE; }

        uint32 GetSplineId() const { return _splineId; }

    private:
        bool i_recalculateSpeed;
        Movement::PointsArray m_precomputedPath;

        uint32 _splineId;
};

#endif

