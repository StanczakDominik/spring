#include "StdAfx.h"
#include "SimpleParticleSystem.h"
#include "GlobalStuff.h"
#include "Rendering/GL/VertexArray.h"
#include "Game/Camera.h"
#include "ProjectileHandler.h"
#include "Rendering/Textures/ColorMap.h"

CR_BIND_DERIVED(CSimpleParticleSystem, CProjectile);

CR_REG_METADATA(CSimpleParticleSystem, 
(
	CR_MEMBER_BEGINFLAG(CM_Config),
		CR_MEMBER(emitVector),
		CR_MEMBER(emitMul),
		CR_MEMBER(gravity),
		CR_MEMBER(colorMap),
		CR_MEMBER(texture),
		CR_MEMBER(airdrag),
		CR_MEMBER(particleLife),
		CR_MEMBER(particleLifeSpread),
		CR_MEMBER(numParticles),
		CR_MEMBER(particleSpeed),
		CR_MEMBER(particleSpeedSpread),
		CR_MEMBER(particleSize),
		CR_MEMBER(particleSizeSpread),
		CR_MEMBER(emitRot),
		CR_MEMBER(emitRotSpread),
		CR_MEMBER(directional),
	CR_MEMBER_ENDFLAG(CM_Config)
));

CSimpleParticleSystem::CSimpleParticleSystem(void)
{
	deleteMe=false;
	checkCol=false;
	useAirLos=true;
	particles=0;
	emitMul = float3(1,1,1);
}

CSimpleParticleSystem::~CSimpleParticleSystem(void)
{
	if(particles)
		delete [] particles;
}

void CSimpleParticleSystem::Draw()
{
	inArray=true;

	if(directional)
	{
		for(int i=0; i<numParticles; i++)
		{
			if(particles[i].life<1.0f)
			{
				float3 dif(particles[i].pos-camera->pos);
				float camDist=dif.Length();
				dif/=camDist;
				float3 dir1(dif.cross(particles[i].speed));
				dir1.Normalize();
				float3 dir2(dif.cross(dir1));

				unsigned char color[4];

				colorMap->GetColor(color, particles[i].life);
				float3 interPos=particles[i].pos+particles[i].speed*gu->timeOffset;
				float size = particles[i].size;
				va->AddVertexTC(interPos-dir1*particleSize-dir2*particleSize,texture->xstart,texture->ystart,color);
				va->AddVertexTC(interPos-dir1*particleSize+dir2*particleSize,texture->xend ,texture->ystart,color);
				va->AddVertexTC(interPos+dir1*particleSize+dir2*particleSize,texture->xend ,texture->yend ,color);
				va->AddVertexTC(interPos+dir1*particleSize-dir2*particleSize,texture->xstart,texture->yend ,color);
			}
		}
	}
	else
	{
		for(int i=0; i<numParticles; i++)
		{
			if(particles[i].life<1.0f)
			{
				unsigned char color[4];

				colorMap->GetColor(color, particles[i].life);
				float3 interPos=particles[i].pos+particles[i].speed*gu->timeOffset;
				float size = particles[i].size;

				va->AddVertexTC(interPos-camera->right*particleSize-camera->up*particleSize,texture->xstart,texture->ystart,color);
				va->AddVertexTC(interPos+camera->right*particleSize-camera->up*particleSize,texture->xend ,texture->ystart,color);
				va->AddVertexTC(interPos+camera->right*particleSize+camera->up*particleSize,texture->xend ,texture->yend ,color);
				va->AddVertexTC(interPos-camera->right*particleSize+camera->up*particleSize,texture->xstart,texture->yend ,color);
			}
		}
	}
}

void CSimpleParticleSystem::Update()
{
	deleteMe = true;

	for(int i=0; i<numParticles; i++)
	{
		if(particles[i].life<1.0f)
		{


			particles[i].pos += particles[i].speed;
			particles[i].life += particles[i].decayrate;
			particles[i].speed += gravity;

			particles[i].speed *= airdrag;

			deleteMe = false;
		}
	}

}

void CSimpleParticleSystem::Init(const float3& explosionPos, CUnit *owner)
{
	CProjectile::Init(explosionPos, owner);
	
	particles = new Particle[numParticles];

	float3 up = emitVector;
	float3 right = up.cross(float3(up.y,up.z,-up.x));
	float3 forward = up.cross(right);

	for(int i=0; i<numParticles; i++)
	{

		float az = gu->usRandFloat()*2*PI;
		float ay = (emitRot + emitRotSpread*gu->usRandFloat())*(PI/180.0);

		particles[i].decayrate = 1.0f/(particleLife + gu->usRandFloat()*particleLifeSpread);
		particles[i].life = 0;
		particles[i].size = particleSize + gu->usRandFloat()*particleSizeSpread;
		particles[i].pos = pos;

		particles[i].speed = ((up*emitMul.y)*cos(ay)-((right*emitMul.x)*cos(az)-(forward*emitMul.z)*sin(az))*sin(ay)) * (particleSpeed + gu->usRandFloat()*particleSpeedSpread);
	}

	drawRadius = (particleSpeed+particleSpeedSpread)*(particleLife*particleLifeSpread);
}
