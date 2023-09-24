//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_portalgun_shared.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "rumble_shared.h"

#include "prop_portal_shared.h"
#include "particle_parse.h"

#ifdef CLIENT_DLL
#include "prediction.h"
#endif

ConVar portalgun_fire_delay ( "portalgun_fire_delay", "0.20", FCVAR_CHEAT|FCVAR_REPLICATED );
ConVar portalgun_held_button_fire_delay ( "portalgun_held_button_fire_fire_delay", "0.50", FCVAR_CHEAT|FCVAR_REPLICATED );

#ifdef CLIENT_DLL
	#define CWeaponPortalgun C_WeaponPortalgun
#endif //#ifdef CLIENT_DLL


acttable_t	CWeaponPortalgun::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_MP_STAND_PRIMARY,					false },
	{ ACT_MP_RUN,						ACT_MP_RUN_PRIMARY,						false },
	{ ACT_MP_CROUCH_IDLE,				ACT_MP_CROUCH_PRIMARY,					false },
	{ ACT_MP_CROUCHWALK,				ACT_MP_CROUCHWALK_PRIMARY,				false },
	{ ACT_MP_JUMP_START,				ACT_MP_JUMP_START_PRIMARY,				false },
	{ ACT_MP_JUMP_FLOAT,				ACT_MP_JUMP_FLOAT_PRIMARY,				false },
	{ ACT_MP_JUMP_LAND,					ACT_MP_JUMP_LAND_PRIMARY,				false },
	{ ACT_MP_AIRWALK,					ACT_MP_AIRWALK_PRIMARY,					false },
};

IMPLEMENT_ACTTABLE(CWeaponPortalgun);


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponPortalgun::CWeaponPortalgun( void )
{
	m_bReloadsSingly = true;

	// TODO: specify these in hammer instead of assuming every gun has blue chip
	m_bCanFirePortal1 = true;
	m_bCanFirePortal2 = false;

	m_iLastFiredPortal = 0;
	m_fCanPlacePortal1OnThisSurface = 1.0f;
	m_fCanPlacePortal2OnThisSurface = 1.0f;

	m_fMinRange1	= 0.0f;
	m_fMaxRange1	= MAX_TRACE_LENGTH;
	m_fMinRange2	= 0.0f;
	m_fMaxRange2	= MAX_TRACE_LENGTH;

	m_EffectState	= EFFECT_NONE;
}

void CWeaponPortalgun::Precache()
{
	BaseClass::Precache();

	PrecacheEffect("PortalBlast");

	PrecacheModel( PORTALGUN_BEAM_SPRITE );
	PrecacheModel( PORTALGUN_BEAM_SPRITE_NOZ );

	PrecacheModel( "models/portals/portal1.mdl" );
	PrecacheModel( "models/portals/portal2.mdl" );

	PrecacheScriptSound( "Portal.ambient_loop" );

	PrecacheScriptSound( "Portal.open_blue" );
	PrecacheScriptSound( "Portal.open_red" );
	PrecacheScriptSound( "Portal.close_blue" );
	PrecacheScriptSound( "Portal.close_red" );
	PrecacheScriptSound( "Portal.fizzle_moved" );
	PrecacheScriptSound( "Portal.fizzle_invalid_surface" );
	PrecacheScriptSound( "Weapon_Portalgun.powerup" );
	PrecacheScriptSound( "Weapon_PhysCannon.HoldSound" );

#ifndef CLIENT_DLL
	PrecacheParticleSystem( "portal_1_projectile_stream" );
	PrecacheParticleSystem( "portal_1_projectile_stream_pedestal" );
	PrecacheParticleSystem( "portal_2_projectile_stream" );
	PrecacheParticleSystem( "portal_2_projectile_stream_pedestal" );
	PrecacheParticleSystem( "portal_1_charge" );
	PrecacheParticleSystem( "portal_2_charge" );
	
	PrecacheParticleSystem( "portal_projectile_stream" );

	PrecacheParticleSystem( "portalgun_muzzleflash_FP" );
	PrecacheParticleSystem( "portalgun_muzzleflash" );
	

#endif
}

PRECACHE_WEAPON_REGISTER(weapon_portalgun);

bool CWeaponPortalgun::ShouldDrawCrosshair( void )
{
	return true;//( m_fCanPlacePortal1OnThisSurface > 0.5f || m_fCanPlacePortal2OnThisSurface > 0.5f );
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponPortalgun::Reload( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponPortalgun::FillClip( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return;

	// Add them to the clip
	if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) > 0 )
	{
		if ( Clip1() < GetMaxClip1() )
		{
			m_iClip1++;
			pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponPortalgun::DryFire( void )
{
	WeaponSound(EMPTY);
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponPortalgun::SetCanFirePortal1( bool bCanFire /*= true*/ )
{
	m_bCanFirePortal1 = bCanFire;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if ( !m_bOpenProngs )
	{
		DoEffect( EFFECT_HOLDING );
		DoEffect( EFFECT_READY );
	}

	// TODO: Remove muzzle flash when there's an upgrade animation
	pOwner->DoMuzzleFlash();

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.25f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.25f;

	// player "shoot" animation
	pOwner->SetAnimation( PLAYER_ATTACK1 );

	pOwner->ViewPunch( QAngle( random->RandomFloat( -1, -0.5f ), random->RandomFloat( -1, 1 ), 0 ) );

	EmitSound( "Weapon_Portalgun.powerup" );
}

void CWeaponPortalgun::SetCanFirePortal2( bool bCanFire /*= true*/ )
{
	m_bCanFirePortal2 = bCanFire;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if ( !m_bOpenProngs )
	{
		DoEffect( EFFECT_HOLDING );
		DoEffect( EFFECT_READY );
	}

	// TODO: Remove muzzle flash when there's an upgrade animation
	pOwner->DoMuzzleFlash();

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;

	// player "shoot" animation
	pOwner->SetAnimation( PLAYER_ATTACK1 );

	pOwner->ViewPunch( QAngle( random->RandomFloat( -1, -0.5f ), random->RandomFloat( -1, 1 ), 0 ) );

	EmitSound( "Weapon_Portalgun.powerup" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponPortalgun::PrimaryAttack( void )
{
	if ( !CanFirePortal1() )
		return;

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

#ifndef CLIENT_DLL
	inputdata_t inputdata;
	inputdata.pActivator = this;
	inputdata.pCaller = this;
	inputdata.value;//null
	FirePortal1( inputdata );
	m_OnFiredPortal1.FireOutput( pPlayer, this );

	pPlayer->RumbleEffect( RUMBLE_PORTALGUN_LEFT, 0, RUMBLE_FLAGS_NONE );
#endif

	pPlayer->DoMuzzleFlash();

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;//SequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;//SequenceDuration();

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	pPlayer->ViewPunch( QAngle( random->RandomFloat( -1, -0.5f ), random->RandomFloat( -1, 1 ), 0 ) );

	PostAttack();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponPortalgun::SecondaryAttack( void )
{
	if ( !CanFirePortal2() )
		return;

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

#ifndef CLIENT_DLL
	inputdata_t inputdata;
	inputdata.pActivator = this;
	inputdata.pCaller = this;
	inputdata.value;//null
	FirePortal2( inputdata );
	m_OnFiredPortal2.FireOutput( pPlayer, this );
	pPlayer->RumbleEffect( RUMBLE_PORTALGUN_RIGHT, 0, RUMBLE_FLAGS_NONE );
#endif

	pPlayer->DoMuzzleFlash();

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;//SequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;//SequenceDuration();

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	pPlayer->ViewPunch( QAngle( random->RandomFloat( -1, -0.5f ), random->RandomFloat( -1, 1 ), 0 ) );

	PostAttack();
}

void CWeaponPortalgun::DelayAttack( float fDelay )
{
	m_flNextPrimaryAttack = gpGlobals->curtime + fDelay;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPortalgun::ItemHolsterFrame( void )
{
	// Must be player held
	if ( GetOwner() && GetOwner()->IsPlayer() == false )
		return;

	// We can't be active
	if ( GetOwner()->GetActiveWeapon() == this )
		return;

#ifndef PORTAL2_DLL
	// If it's been longer than three seconds, reload
	if ( ( gpGlobals->curtime - m_flHolsterTime ) > sk_auto_reload_time.GetFloat() )
	{
		// Reset the timer
		m_flHolsterTime = gpGlobals->curtime;
	
		if ( GetOwner() == NULL )
			return;

		if ( m_iClip1 == GetMaxClip1() )
			return;

		// Just load the clip with no animations
		int ammoFill = MIN( (GetMaxClip1() - m_iClip1), GetOwner()->GetAmmoCount( GetPrimaryAmmoType() ) );
		
		GetOwner()->RemoveAmmo( ammoFill, GetPrimaryAmmoType() );
		m_iClip1 += ammoFill;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPortalgun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	DestroyEffects();

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPortalgun::Deploy( void )
{
	DoEffect( EFFECT_READY );

	bool bReturn = BaseClass::Deploy();

	m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner )
	{
		pOwner->SetNextAttack( gpGlobals->curtime );

#ifndef CLIENT_DLL
		if( GameRules()->IsMultiplayer() )
		{
			m_iPortalLinkageGroupID = pOwner->entindex();

			Assert( (m_iPortalLinkageGroupID >= 0) && (m_iPortalLinkageGroupID < 256) );
		}
#endif
	}

	return bReturn;
}

void CWeaponPortalgun::WeaponIdle( void )
{
	//See if we should idle high or low
	if ( WeaponShouldBeLowered() )
	{
		// Move to lowered position if we're not there yet
		if ( GetActivity() != ACT_VM_IDLE_LOWERED && GetActivity() != ACT_VM_IDLE_TO_LOWERED 
			&& GetActivity() != ACT_TRANSITION )
		{
			SendWeaponAnim( ACT_VM_IDLE_LOWERED );
		}
		else if ( HasWeaponIdleTimeElapsed() )
		{
			// Keep idling low
			SendWeaponAnim( ACT_VM_IDLE_LOWERED );
		}
	}
	else
	{
		// See if we need to raise immediately
		if ( m_flRaiseTime < gpGlobals->curtime && GetActivity() == ACT_VM_IDLE_LOWERED ) 
		{
			SendWeaponAnim( ACT_VM_IDLE );
		}
		else if ( HasWeaponIdleTimeElapsed() ) 
		{
			SendWeaponAnim( ACT_VM_IDLE );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPortalgun::StopEffects( bool stopSound )
{
	// Turn off our effect state
	DoEffect( EFFECT_NONE );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : effectType - 
//-----------------------------------------------------------------------------
void CWeaponPortalgun::DoEffect( int effectType, Vector *pos )
{
	m_EffectState = effectType;

#ifdef CLIENT_DLL
	// Save predicted state
	m_nOldEffectState = m_EffectState;
#endif

	switch( effectType )
	{
	case EFFECT_READY:
		DoEffectReady();
		break;

	case EFFECT_HOLDING:
		DoEffectHolding();
		break;

	default:
	case EFFECT_NONE:
		DoEffectNone();
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Restore
//-----------------------------------------------------------------------------
void CWeaponPortalgun::OnRestore()
{
	BaseClass::OnRestore();

	// Portalgun effects disappear through level transition, so
	//  just recreate any effects here
	if ( m_EffectState != EFFECT_NONE )
	{
		DoEffect( m_EffectState, NULL );
	}
}


//-----------------------------------------------------------------------------
// On Remove
//-----------------------------------------------------------------------------
void CWeaponPortalgun::UpdateOnRemove(void)
{
	DestroyEffects();
	BaseClass::UpdateOnRemove();
}

void CWeaponPortalgun::PostAttack( void )
{
	// Only the player fires this way so we can cast
	CPortal_Player *pPlayer = ToPortalPlayer( GetOwner() );
	if ( pPlayer == NULL )
		return;

#if defined( GAME_DLL ) //TODO: client version would probably be a good idea
	pPlayer->RumbleEffect( RUMBLE_PORTALGUN_LEFT, 0, RUMBLE_FLAGS_NONE );
#endif


	int nSplitScreenSlot = pPlayer->GetSplitScreenPlayerSlot();
	CBaseAnimating *pModelView =  pPlayer->GetViewModel();

#if defined( CLIENT_DLL )
	if ( !prediction->InPrediction() || prediction->IsFirstTimePredicted() )
#endif
	{		
		if ( ( pPlayer->IsSplitScreenPlayer() || pPlayer->HasAttachedSplitScreenPlayers() ) && nSplitScreenSlot != -1 )
		{
#if defined( CLIENT_DLL )
			if ( pModelView )
			{
				CUtlReference<CNewParticleEffect> m_hPortalGunMuzzle = pModelView->ParticleProp()->Create( "portalgun_muzzleflash_FP", PATTACH_POINT_FOLLOW, "muzzle" );
				m_hPortalGunMuzzle->SetDrawOnlyForSplitScreenUser( nSplitScreenSlot );
				m_hPortalGunMuzzle = NULL;
			}
			DispatchParticleEffect( "portalgun_muzzleflash", PATTACH_POINT_FOLLOW, this, "muzzle");
#endif
		}
		else
		{
			if ( pModelView )
				DispatchParticleEffect( "portalgun_muzzleflash_FP", PATTACH_POINT_FOLLOW, pModelView, "muzzle" );
			DispatchParticleEffect( "portalgun_muzzleflash", PATTACH_POINT_FOLLOW, this, "muzzle");
		}
	}

	float flFireDelay = portalgun_fire_delay.GetFloat();

#if USE_SLOWTIME
	if ( pPlayer->IsSlowingTime() )
	{
		flFireDelay *= slowtime_speed.GetFloat();
	}
	else
#endif // USE_SLOWTIME
	{
		QAngle qPunch;
		qPunch.x = SharedRandomFloat( "CWeaponPortalgun::PrimaryAttack() ViewPunchX", -1, -0.5f );
		qPunch.y = SharedRandomFloat( "CWeaponPortalgun::PrimaryAttack() ViewPunchY", -1, 1 );
		qPunch.z = 0.0f;
		pPlayer->ViewPunch( qPunch );
	}

	// Don't fire again too quickly
	m_flNextPrimaryAttack = gpGlobals->curtime + flFireDelay;
	m_flNextSecondaryAttack = gpGlobals->curtime + flFireDelay;

	// Held-button repeat fires get a different delay
	m_flNextRepeatPrimaryAttack = gpGlobals->curtime + portalgun_held_button_fire_delay.GetFloat();
	m_flNextRepeatSecondaryAttack = gpGlobals->curtime + portalgun_held_button_fire_delay.GetFloat();
	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
}
