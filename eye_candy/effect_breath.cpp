#ifdef EYE_CANDY

// I N C L U D E S ////////////////////////////////////////////////////////////

#include "eye_candy.h"
#include "math_cache.h"

#include "effect_breath.h"

namespace ec
{

extern MathCache_Lorange math_cache;

// C L A S S   F U N C T I O N S //////////////////////////////////////////////

BreathParticle::BreathParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, const color_t red, const color_t green, const color_t blue, Texture* _texture, const Uint16 _LOD, const BreathEffect::BreathType _type) : Particle(_effect, _mover, _pos, _velocity)
{
  type = _type;
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  texture = _texture;
  size = _size * (0.2 + randcoord()) * 15 / _LOD;
//  alpha = _alpha;
  alpha = _alpha * 2 / size;
  flare_max = 8.0;
  flare_exp = 0.5;
  flare_frequency = 3.0;
  LOD = _LOD;
  state = 0;
}

bool BreathParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;
    
  const interval_t float_time = delta_t / 1000000.0;
  velocity *= math_cache.powf_05_close(float_time * velocity.magnitude() / 8.0);
  
  switch(type)
  {
    case BreathEffect::FIRE:	// Fall through; these cases are all very similar in terms of behavior.
    case BreathEffect::ICE:
    case BreathEffect::POISON:
    case BreathEffect::MAGIC:
    {
      if (state == 0)
      {
        if ((get_time() - born > (type == BreathEffect::POISON ? 100000 : 400000)) || (math_cache.powf_0_1_rough_close(randfloat(), float_time * 5.0)) < 0.5)
          state = 1;
      }
      else
      {
        if (alpha < 0.02)
          return false;
        
        if ((state == 1) && (alpha < 0.04))
        {
          state = 2;
          if (rand() & 0x03 == 0x03)
          {
            Vec3 velocity_offset;
            velocity_offset.randomize(0.2);
            base->push_back_particle(new BreathSmokeParticle(effect, mover, pos, velocity + velocity_offset, size * 2.5, 0.06, texture, LOD, type));
          }
        }

        const alpha_t scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 5.0);
        alpha *= scalar;

        const coord_t size_scalar = math_cache.powf_05_close((float)delta_t / 1500000);
        size = size / size_scalar * 0.25 + size * 0.75;
        if (size >= 5.0)
          size = 5.0;
      }
      break;
    }
    case BreathEffect::WIND:
    {
      if (state == 0)
      {
        if ((get_time() - born > 400000) || (math_cache.powf_0_1_rough_close(randfloat(), float_time * 70.0)) < 0.5)
          state = 1;
      }
      else
      {
        if (alpha < 0.02)
          return false;
        
        if ((state == 1) && (alpha < 0.04))
        {
          state = 2;
          if (rand() & 0x07 == 0x07)
          {
            Vec3 velocity_offset;
            velocity_offset.randomize(1.0);
            base->push_back_particle(new BreathSmokeParticle(effect, mover, pos, velocity + velocity_offset, size * 10.0, 1.0, texture, LOD, type));
          }
        }

        const alpha_t scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 20.0);
        alpha *= scalar;

        const coord_t size_scalar = math_cache.powf_05_close((float)delta_t / 1500000);
        size = size / size_scalar * 0.25 + size * 0.75;
        if (size >= 5.0)
          size = 5.0;
      }
      break;
    }
    case BreathEffect::LIGHTNING:
    {
      if (state == 0)
      {
        if ((get_time() - born > 400000) || (math_cache.powf_0_1_rough_close(randfloat(), float_time * 10.0)) < 0.5)
          state = 1;
      }
      else
      {
        if (alpha < 0.02)
          return false;
        
        if ((state == 1) && (alpha < 0.04))
        {
          state = 2;
          if (rand() & 0x3F == 0x3F)
          {
            Vec3 velocity_offset;
            velocity_offset.randomize();
            velocity_offset.normalize();
            Particle* p = new BreathParticle(effect, mover, pos, velocity_offset * 3.6, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
            p->state = 2;
            base->push_back_particle(p);
            p = new BreathParticle(effect, mover, pos, velocity_offset * 3.4, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
            p->state = 2;
            base->push_back_particle(p);
            p = new BreathParticle(effect, mover, pos, velocity_offset * 3.2, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
            p->state = 2;
            base->push_back_particle(p);
            p = new BreathParticle(effect, mover, pos, velocity_offset * 3.0, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
            p->state = 2;
            base->push_back_particle(p);
            p = new BreathParticle(effect, mover, pos, velocity_offset * 2.8, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
            p->state = 2;
            base->push_back_particle(p);
            p = new BreathParticle(effect, mover, pos, velocity_offset * 2.6, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
            p->state = 2;
            base->push_back_particle(p);
            p = new BreathParticle(effect, mover, pos, velocity_offset * 2.4, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
            p->state = 2;
            base->push_back_particle(p);
            p = new BreathParticle(effect, mover, pos, velocity_offset * 2.2, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
            p->state = 2;
            base->push_back_particle(p);
            p = new BreathParticle(effect, mover, pos, velocity_offset * 2.0, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
            p->state = 2;
            base->push_back_particle(p);
            p = new BreathParticle(effect, mover, pos, velocity_offset * 1.8, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
            p->state = 2;
            base->push_back_particle(p);
            p = new BreathParticle(effect, mover, pos, velocity_offset * 1.6, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
            p->state = 2;
            base->push_back_particle(p);
            p = new BreathParticle(effect, mover, pos, velocity_offset * 1.4, size / 2, 1.0, color[0], color[1], color[2], &(base->TexShimmer), LOD, type);
            p->state = 2;
            base->push_back_particle(p);
          }
        }

        const alpha_t scalar = math_cache.powf_0_1_rough_close(randfloat(), float_time * 5.0);
        alpha *= scalar;
        if (state == 2)
          alpha *= square(scalar);

        const coord_t size_scalar = math_cache.powf_05_close((float)delta_t / 1500000);
        size = size / size_scalar * 0.25 + size * 0.75;
        if (size >= 5.0)
          size = 5.0;
      }
      break;
    }
  }
  
  return true;
}

GLuint BreathParticle::get_texture(const Uint16 res_index)
{
  return texture->get_texture(res_index);
}

BreathSmokeParticle::BreathSmokeParticle(Effect* _effect, ParticleMover* _mover, const Vec3 _pos, const Vec3 _velocity, const coord_t _size, const alpha_t _alpha, Texture* _texture, const Uint16 _LOD, const BreathEffect::BreathType _type) : Particle(_effect, _mover, _pos, _velocity)
{
  texture = _texture;
  type = _type;
  switch (type)
  {
    case BreathEffect::FIRE:
    {
      const color_t color_scale = randcolor(0.1);
      color[0] = randcolor(0.1) + color_scale;
      color[1] = randcolor(0.1) + color_scale;
      color[2] = randcolor(0.1) + color_scale;
      break;
    }
    case BreathEffect::ICE:
    {
      const color_t color_scale = randcolor(0.2);
      color[0] = 0.3 + randcolor(0.1) + color_scale;
      color[1] = 0.3 + randcolor(0.1) + color_scale;
      color[2] = 1.0;
      break;
    }
    case BreathEffect::POISON:
    {
      const color_t color_scale = randcolor(0.1);
      color[0] = randcolor(0.0) + color_scale;
      color[1] = randcolor(0.0) + color_scale;
      color[2] = randcolor(0.0) + color_scale;
      texture = &(base->TexSimple);
      break;
    }
    case BreathEffect::MAGIC:
    {
      color[0] = randcolor(0.35);
      color[1] = randcolor(0.35);
      color[2] = randcolor(0.35);
      break;
    }
    case BreathEffect::LIGHTNING:	// Impossible; lightning doesn't use smoke.
    {
      break;
    }
    case BreathEffect::WIND:
    {
      const color_t color_scale = randcolor(0.4);
      color[0] = randcolor(0.1) + color_scale;
      color[1] = randcolor(0.1) + color_scale;
      color[2] = randcolor(0.1) + color_scale;
      break;
    }
  }
  size = _size * (0.5 + randcoord()) * 10 / _LOD;
  alpha = _alpha;
  flare_max = 1.0;
  flare_exp = 1.0;
  flare_frequency = 1.0;
  state = 0;
}

void BreathSmokeParticle::draw(const Uint64 usec)
{
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Particle::draw(usec);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
}

bool BreathSmokeParticle::idle(const Uint64 delta_t)
{
  if (effect->recall)
    return false;

  const interval_t float_time = delta_t / 1000000.0;
  velocity *= math_cache.powf_05_close(float_time * velocity.magnitude() / 2.0);
  
  if (state == 0)
  {
    alpha += float_time * 2.0;
  
    if (alpha >= 0.4)
    {
      alpha = 0.4;
      state = 1;
    }
  }
  else
  {
    const alpha_t alpha_scalar = 1.0 - math_cache.powf_05_close((float)delta_t / 1000000);
    alpha -= alpha_scalar;

    if (alpha < 0.005)
      return false;
  
    const coord_t size_scalar = math_cache.powf_05_close((float)delta_t / 1500000);
    size = size / size_scalar * 0.25 + size * 0.75;
    if (size >= 5)
      size = 5;
  }

  return true;
}

GLuint BreathSmokeParticle::get_texture(const Uint16 res_index)
{
  return texture->get_texture(res_index);
}

BreathEffect::BreathEffect(EyeCandy* _base, bool* _dead, Vec3* _pos, Vec3* _target, const std::vector<ec::Obstruction*> _obstructions, const BreathType _type, const Uint16 _LOD, const percent_t _scale)
{
  if (EC_DEBUG)
    std::cout << "BreathEffect (" << this << ") created." << std::endl;
  base = _base;
  dead = _dead;
  pos = _pos;
  target = _target;
  obstructions = _obstructions;
  type = _type;
  LOD = _LOD;
  desired_LOD = _LOD;
  scale = _scale;
  spawner = NULL;
  mover = NULL;
  count = 0;
  count_scalar = 3000 / LOD;
  size_scalar = scale * fastsqrt(LOD) / sqrt(10.0);
  
  spawner = new FilledSphereSpawner(scale / 3.0);
  mover = new SmokeMover(this, 10.0);
  while ((int)particles.size() < LOD * 250)
  {
    Vec3 coords = spawner->get_new_coords() + *pos;
    Vec3 velocity;
    velocity.randomize(0.5);
    velocity += (*target - *pos) * 2.0;
    switch(type)
    {
      case FIRE:
      {
        Particle * p = new BreathParticle(this, mover, coords, velocity, 1.5 * size_scalar, 0.6, 0.8 + randcolor(0.2), 0.4 + randcolor(0.4), randcolor(0.4), &(base->TexFlare), LOD, type);
        if (!base->push_back_particle(p))
          return;
        break;
      }
      case ICE:
      {
        Particle * p = new BreathParticle(this, mover, coords, velocity, 2.0 * size_scalar, 0.6, randcolor(0.4), 0.4 + randcolor(0.4), 0.8 + randcolor(0.2), &(base->TexCrystal), LOD, type);
        if (!base->push_back_particle(p))
          return;
        break;
      }
      case POISON:
      {
        Particle * p = new BreathParticle(this, mover, coords, velocity, 1.5 * size_scalar, 0.1, randcolor(0.3), 0.8 + randcolor(0.2), randcolor(0.3), &(base->TexWater), LOD, type);
        if (!base->push_back_particle(p))
          return;
        break;
      }
      case MAGIC:
      {
        Particle * p = new BreathParticle(this, mover, coords, velocity, 1.5 * size_scalar, 0.6, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexTwinflare), LOD, type);
        if (!base->push_back_particle(p))
          return;
        break;
      }
      case LIGHTNING:
      {
        Particle * p = new BreathParticle(this, mover, coords, velocity, 0.75 * size_scalar, 0.6, 0.9 + randcolor(0.1), 0.85 + randcolor(0.15), 0.8 + randcolor(0.2), &(base->TexFlare), LOD, type);
        if (!base->push_back_particle(p))
          return;
        break;
      }
      case WIND:
      {
        Particle * p = new BreathParticle(this, mover, coords, velocity, 1.1 * size_scalar, 0.6, 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), &(base->TexFlare), LOD, type);
        if (!base->push_back_particle(p))
          return;
        break;
      }
    }
  }
}

BreathEffect::~BreathEffect()
{
  if (spawner)
    delete spawner;
  if (mover)
    delete mover;
  if (EC_DEBUG)
    std::cout << "BreathEffect (" << this << ") destroyed." << std::endl;
}

bool BreathEffect::idle(const Uint64 usec)
{
  if (particles.size() == 0)
    return false;
  
  if (recall)
    return true;
    
  const Uint64 cur_time = get_time();
  const Uint64 age = cur_time - born;
  if (age > 1100000)
    return true;

  count += usec;

  while (count > 0)
  {
    count -= count_scalar;

    Vec3 coords;
    Vec3 velocity;
    if (type == WIND)
    {
      coords = spawner->get_new_coords() * 2 + *pos;
      velocity.randomize(1.0 * scale);
      velocity += (*target - *pos) * 10.0;
    }
    else
    {
      coords = spawner->get_new_coords() + *pos;
      velocity.randomize(0.5 * scale);
      velocity += (*target - *pos) * 2.0;
    }

    switch(type)
    {
      case FIRE:
      {
        Particle * p = new BreathParticle(this, mover, coords, velocity, 1.5 * size_scalar, 0.6, 0.8 + randcolor(0.2), 0.4 + randcolor(0.4), randcolor(0.4), &(base->TexFlare), LOD, type);
        if (!base->push_back_particle(p))
          return true;
        break;
      }
      case ICE:
      {
        Particle * p = new BreathParticle(this, mover, coords, velocity, 2.0 * size_scalar, 0.6, randcolor(0.4), 0.4 + randcolor(0.4), 0.8 + randcolor(0.2), &(base->TexCrystal), LOD, type);
        if (!base->push_back_particle(p))
          return true;
        break;
      }
      case POISON:
      {
        Particle * p = new BreathParticle(this, mover, coords, velocity, 1.5 * size_scalar, 0.1, 0.9 + randcolor(0.1), 0.8 + randcolor(0.2), 0.6 + randcolor(0.2), &(base->TexFlare), LOD, type);
        if (!base->push_back_particle(p))
          return true;
        break;
      }
      case MAGIC:
      {
        Particle * p = new BreathParticle(this, mover, coords, velocity, 1.5 * size_scalar, 0.6, randcolor(1.0), randcolor(1.0), randcolor(1.0), &(base->TexTwinflare), LOD, type);
        if (!base->push_back_particle(p))
          return true;
        break;
      }
      case LIGHTNING:
      {
        Particle * p = new BreathParticle(this, mover, coords, velocity, 1.1 * size_scalar, 0.6, 0.9 + randcolor(0.1), 0.85 + randcolor(0.15), 0.8 + randcolor(0.2), &(base->TexFlare), LOD, type);
        if (!base->push_back_particle(p))
          return true;
        break;
      }
      case WIND:
      {
        Particle * p = new BreathParticle(this, mover, coords, velocity, 1.1 * size_scalar, 0.1, 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), 0.8 + randcolor(0.2), &(base->TexFlare), LOD, type);
        if (!base->push_back_particle(p))
          return true;
        break;
      }
    }
  }
  
  return true;
}

///////////////////////////////////////////////////////////////////////////////

};

#endif	// #ifdef EYE_CANDY