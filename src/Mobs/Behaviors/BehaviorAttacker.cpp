
#include "Globals.h"  // NOTE: MSVC stupidness requires this to be the same across all modules

#include "BehaviorAttacker.h"
#include "BehaviorStriker.h"
#include "../Monster.h"
#include "../../Entities/Pawn.h"
#include "../../Entities/Player.h"
#include "../../Tracer.h"


cBehaviorAttacker::cBehaviorAttacker() :
	m_AttackRate(3)
  , m_AttackDamage(1)
  , m_AttackRange(1)
  , m_AttackCoolDownTicksLeft(0)
  , m_TicksSinceLastDamaged(100)
{

}





void cBehaviorAttacker::AttachToMonster(cMonster & a_Parent, cBehaviorStriker & a_ParentStriker)
{
	m_Parent = &a_Parent;
	m_ParentStriker = &a_ParentStriker;
	m_Parent->AttachTickBehavior(this);
	m_Parent->AttachDestroyBehavior(this);
	m_Parent->AttachPostTickBehavior(this);
	m_Parent->AttachDoTakeDamageBehavior(this);
}





bool cBehaviorAttacker::IsControlDesired(std::chrono::milliseconds a_Dt, cChunk & a_Chunk)
{
	UNUSED(a_Dt);
	UNUSED(a_Chunk);
	// If we have a target, we have something to do! Return true and control the mob Ticks.
	// Otherwise return false.
	return (GetTarget() != nullptr);
}





void cBehaviorAttacker::Tick(std::chrono::milliseconds a_Dt, cChunk & a_Chunk)
{
	UNUSED(a_Dt);
	UNUSED(a_Chunk);
	/*
	 * 	if ((GetTarget() != nullptr))
	{
		ASSERT(GetTarget()->IsTicking());

		if (GetTarget()->IsPlayer())
		{
			if (!static_cast<cPlayer *>(GetTarget())->CanMobsTarget())
			{
				SetTarget(nullptr);
			}
		}
	} //mobTodo copied from monster.cpp
	 * */

	ASSERT((GetTarget() == nullptr) || (GetTarget()->IsPawn() && (GetTarget()->GetWorld() == m_Parent->GetWorld())));
	// Stop targeting out of range targets
	if (GetTarget() != nullptr)
	{
		if (TargetOutOfSight())
		{
			SetTarget(nullptr);
		}
		else
		{
			if (TargetIsInStrikeRange())
			{
				StrikeTarget();
			}
			else
			{
				ApproachTarget(); // potential mobTodo: decoupling approaching from attacking
				// Not important now, but important for future extensibility, e.g.
				// cow chases wheat but using the netherman approacher to teleport around.
			}
		}
	}
}

void cBehaviorAttacker::ApproachTarget()
{
	m_Parent->MoveToPosition(m_Target->GetPosition());
}



void cBehaviorAttacker::PostTick(std::chrono::milliseconds a_Dt, cChunk & a_Chunk)
{
	if (m_TicksSinceLastDamaged < 100)
	{
		++m_TicksSinceLastDamaged;
	}

	if (m_AttackCoolDownTicksLeft > 0)
	{
		m_AttackCoolDownTicksLeft -= 1;
	}
}





void cBehaviorAttacker::DoTakeDamage(TakeDamageInfo & a_TDI)
{
	if ((a_TDI.Attacker != nullptr) && a_TDI.Attacker->IsPawn())
	{
		if (
			(!a_TDI.Attacker->IsPlayer()) ||
			(static_cast<cPlayer *>(a_TDI.Attacker)->CanMobsTarget())
		)
		{
			SetTarget(static_cast<cPawn*>(a_TDI.Attacker));
		}
		m_TicksSinceLastDamaged = 0;
	}
}





void cBehaviorAttacker::Destroyed()
{
	SetTarget(nullptr);
}





void cBehaviorAttacker::SetAttackRate(float a_AttackRate)
{
	m_AttackRate = a_AttackRate;
}





void cBehaviorAttacker::SetAttackRange(int a_AttackRange)
{
	m_AttackRange = a_AttackRange;
}





void cBehaviorAttacker::SetAttackDamage(int a_AttackDamage)
{
	m_AttackDamage = a_AttackDamage;
}




cPawn * cBehaviorAttacker::GetTarget()
{
	return m_Target;
}





void cBehaviorAttacker::SetTarget(cPawn * a_Target)
{
	m_Target = a_Target;
}





bool cBehaviorAttacker::TargetIsInStrikeRange()
{
	ASSERT(m_Target != nullptr);
	ASSERT(m_Parent != nullptr);


	cTracer LineOfSight(m_Parent->GetWorld());
	Vector3d MyHeadPosition = m_Parent->GetPosition() + Vector3d(0, m_Parent->GetHeight(), 0);
	Vector3d AttackDirection(GetTarget()->GetPosition() + Vector3d(0, GetTarget()->GetHeight(), 0) - MyHeadPosition);

	if (TargetIsInRange() && !LineOfSight.Trace(MyHeadPosition, AttackDirection, static_cast<int>(AttackDirection.Length())) && (GetHealth() > 0.0))
	{
		// Attack if reached destination, target isn't null, and have a clear line of sight to target (so won't attack through walls)
		Attack(a_Dt);
	}

	return ((m_Target->GetPosition() - m_Parent->GetPosition()).SqrLength() < (m_AttackRange * m_AttackRange));
}





bool cBehaviorAttacker::TargetOutOfSight()
{
	ASSERT(m_Target != nullptr);
	if ((GetTarget()->GetPosition() - m_Parent->GetPosition()).Length() > m_Parent->GetSightDistance())
	{
		return true;
	}
	return false;
}





void cBehaviorAttacker::ResetStrikeCooldown()
{
	m_AttackCoolDownTicksLeft = static_cast<int>(3 * 20 * m_AttackRate);  // A second has 20 ticks, an attack rate of 1 means 1 hit every 3 seconds
}





void cBehaviorAttacker::StrikeTarget()
{
	if (m_AttackCoolDownTicksLeft != 0)
	{
		cBehaviorStriker * Striker = m_ParentStriker;
		if (Striker != nullptr)
		{
			// Striker->Strike(m_Target); //mobTodo
		}
		ResetStrikeCooldown();
	}
}
