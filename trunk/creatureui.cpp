//////////////////////////////////////////////////////////////////////
// Yet Another Tibia Client
//////////////////////////////////////////////////////////////////////
// Creature instance class
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

// TODO (ivucica#2#) review code added in 2739 which was written by mrauter, it's probably not 100% the way we want to do things, plus we need colorization

#include "creatureui.h"
#include "engine.h"
#include "fassert.h"
#include "gamecontent/creature.h"
#include "gamecontent/globalvars.h"
#include "options.h"
#include <cmath>

static std::map<uint32_t, Sprite*> s_spritecache;
extern uint32_t g_frameTime;
extern uint32_t g_frameDiff;

CreatureUI::CreatureUI() : ThingUI()
{
	//m_gfx.insert(m_gfx.end(), g_engine->createSprite("Tibia.spr", Objects::getInstance()->getOutfitType(1)->imageData[0]));
	resetSelf();
}

CreatureUI::~CreatureUI()
{
	this->unloadGfx();
	m_obj = NULL;
}

void CreatureUI::resetSelf()
{
	unloadGfx();

	m_preWalk = false;
	m_walkState = 1.f;
}
void CreatureUI::unloadGfx()
{
	for (std::vector<Sprite*>::iterator it = m_gfx.begin(); it != m_gfx.end(); it++){
		delete *it;
	}
	m_gfx.clear();
	m_obj = NULL;
}

void CreatureUI::Blit(int x, int y, float scale, int map_x, int map_y) const
{

    if (!m_obj)
		return;

	if(map_x != 0 && map_y != 0) {
		x = x - std::floor(m_obj->xOffset * scale);
		y = y - std::floor(m_obj->yOffset * scale);
	}
	else {
		// shrink larger creatures to be 32x32
		scale /= MAX(m_obj->width, m_obj->height);
	}

    Creature* n = (Creature*)this;


    if (!isLoaded()) {
        printf("Not loaded!\n");
        return;
    }

    uint32_t activeframe = 0;

    uint32_t spriteSize, partSize, aniSize;
	spriteSize = m_obj->width * m_obj->height * m_obj->blendframes;
	partSize = spriteSize * m_obj->xdiv;
	aniSize = partSize * m_obj->ydiv;

    if(n->getOutfit().m_looktype != 0){
    	if(map_x != 0 && map_y != 0)
			activeframe = ((m_walkState == 1. && !isPreWalking()) ? n->getTurnDir() :  n->getLookDir()) * spriteSize; // creature must have different turning direction and looking (moving) direction... if moving, moving direction takes precendence, if standing, turning direction takes precendence
		else
			activeframe = DIRECTION_SOUTH * spriteSize;
    }


	float walkoffx = 0.f, walkoffy = 0.f;
	if(map_x != 0 && map_y != 0) {
		getWalkOffset(walkoffx, walkoffy, scale);
	}

	//for(uint32_t k = 0; k < m_obj->blendframes; ++k){ // note: if it's anything except item, there won't be blendframes...
	{
	    uint32_t aframes;
	    if(m_obj->animcount == 3) // TODO (ivucica#2#) add support for permanently moving creatures (e.g. rotworms) and for item-based outfits with animcount != 3
			aframes = aniSize * (m_walkState == 1. ? 0 : (((int)(m_walkState*100) / 25) % 2 + 1));
		else
			aframes = 0;

		//Square around the creature
		if(g_frameTime - n->getSquareStart() < 1000){
			g_engine->drawRectangleLines(x + walkoffx, y + walkoffy, 32*scale, 32*scale, oRGBA(0,0,0,0));
		}

		//Pink square around the attacked creature
		if(n->getID() == GlobalVariables::getAttackID()) {
			// "Deep Pink" - may not be the right color
			g_engine->drawRectangleLines((x+1) + walkoffx, (y+1) + walkoffy, 32*scale, 32*scale, oRGBA(255, 140, 7, 255), 2*scale);
		}
		//Green square around the followed creature
		else if(n->getID() == GlobalVariables::getFollowID()) {
			// "Lime Green" - may not be the right color
			g_engine->drawRectangleLines((x+2) + walkoffx, (y+2) + walkoffy, 32*scale, 32*scale, oRGBA(50, 205, 50, 255), 2*scale);
		}

		// "Creature Squares" sent by the server
		// NOTE (nfries88): I have no idea if this will render the proper colors at all.
		// It does send black square correctly, though.
		// Also not sure how long the square is supposed to be shown... 1s? 500ms?
		if((map_x != 0 && map_y != 0) && (n->getSquareStart() + 1000) >= g_frameTime)
		{
			g_engine->drawRectangleLines(x + walkoffx, y + walkoffy, 34*scale, 34*scale, n->getSquareColor(), 2*scale);
		}

		for(uint32_t i = 0; i < m_obj->height; ++i){
			for(uint32_t j = 0; j < m_obj->width; ++j){

				ASSERT(activeframe + aframes < m_obj->numsprites);

				int draw_x = (int)(x - j*32*scale + walkoffx);
				int draw_y = (int)(y - i*32*scale + walkoffy);

                if (m_gfx[activeframe + aframes])
                    m_gfx[activeframe + aframes]->Blit(draw_x, draw_y, 0, 0, 32, 32, 32*scale, 32*scale);
                else
                    printf("Warning: rendering creature failed since sprite %d is NULL\n", activeframe+aframes);

                if(m_obj->ydiv != 1){
                	if(n->getOutfit().m_addons & 1){
                    	m_gfx[activeframe + partSize + aframes]->Blit(draw_x, draw_y, 0, 0, 32, 32, 32*scale, 32*scale);
                	}
                	if(n->getOutfit().m_addons & 2){
                    	m_gfx[activeframe + partSize*2 + aframes]->Blit(draw_x, draw_y, 0, 0, 32, 32, 32*scale, 32*scale);
                	}
                }

				activeframe++;
			}
		}
	}
}

void CreatureUI::drawName(int x, int y, float scale) const
{

    if (!options.shownames) return;
	float walkoffx = 0.f, walkoffy = 0.f;


	Creature* n = (Creature*)this;
	Outfit_t outfit = n->getOutfit();
	if(!m_obj){
		return;
	}


    std::string name = n->getName().c_str();
    name[0] = toupper(name[0]);

	volatile float centralizationoffset = -(g_engine->sizeText( name.c_str(), "gamefont" ) / 2) + 16 - 8;
	getWalkOffset(walkoffx, walkoffy, scale);

    int hp = n->getHealth();
    oRGBA col = getHealthColor(hp);

	g_engine->drawText(name.c_str() , "gamefont", (int)(x + walkoffx + centralizationoffset), (int)(y - 16 - 8 + walkoffy), col);

	g_engine->drawRectangle(x + walkoffx + centralizationoffset, y - 14 + walkoffy, 35, 4, oRGBA(0,0,0,1));
	g_engine->drawRectangle(x + walkoffx + centralizationoffset+1, y - 13 + walkoffy, 33*(hp/100.), 2, col);
}

oRGBA CreatureUI::getHealthColor(int hp)
{
	oRGBA col;
    if (hp >= 50.0) {
        col.r = (1. - (hp/200.))*255.;
        col.g = (0.75 + hp/400.)*255.;
        col.b = 0.;
        col.a = 255.;
    } else {
        col.r = 255.;
        col.g = (hp / 50.) * 255.;
        col.b = 0.;
        col.a = 1.;
    }
    return col;
}

void CreatureUI::drawSkullsShields(int x, int y, float scale) const
{
	// skulls: (54, 225), each skull 11x11, green yellow white red black
	// shields: (54, 236), each shield 11x11, yellow blue whiteyellow whiteblue
	// shields: (76, 214), each shield 11x11, yellowsharedexp bluesharedexp
	// shields: (168, 261), each shield 11x11, yellownosharedexp (+ blinking) bluenosharedexp (+ blinking)
	// emblems: (287, 211), each emblem 11x11, green red blue

	Creature* n = (Creature*)this;
	float walkoffx = 0.f, walkoffy = 0.f;

	getWalkOffset(walkoffx, walkoffy, scale);

	x+=walkoffx+11;
	y+=walkoffy-10;
	uint32_t skull =  n->getSkull();
	switch (skull) {
		case SKULL_GREEN:
			g_engine->getUISprite()->Blit(x, y-10, 54, 225, 11, 11);
			break;
		case SKULL_YELLOW:
			g_engine->getUISprite()->Blit(x, y-10, 65, 225, 11, 11);
			break;
		case SKULL_WHITE:
			g_engine->getUISprite()->Blit(x, y-10, 76, 225, 11, 11);
			break;
		case SKULL_RED:
			g_engine->getUISprite()->Blit(x, y-10, 87, 225, 11, 11);
			break;
		case SKULL_BLACK:
			g_engine->getUISprite()->Blit(x, y-10, 98, 297, 11, 11);
			break;

		default: break;
	}

	uint32_t shield =  n->getShield();
	switch(shield) {
		case SHIELD_YELLOW:
			g_engine->getUISprite()->Blit(x-11, y-10, 54, 236, 11, 11);
			break;
		case SHIELD_BLUE:
			g_engine->getUISprite()->Blit(x-11, y-10, 65, 236, 11, 11);
			break;
		case SHIELD_WHITEYELLOW:
			g_engine->getUISprite()->Blit(x-11, y-10, 76, 236, 11, 11);
			break;
		case SHIELD_WHITEBLUE:
			g_engine->getUISprite()->Blit(x-11, y-10, 87, 236, 11, 11);
			break;

		case SHIELD_YELLOW_SHAREDEXP:
			g_engine->getUISprite()->Blit(x-11, y-10, 76, 214, 11, 11);
			break;
		case SHIELD_BLUE_SHAREDEXP:
			g_engine->getUISprite()->Blit(x-11, y-10, 87, 214, 11, 11);
			break;
		case SHIELD_YELLOW_NOSHAREDEXP_BLINK:
			// TODO (nfries88): actually make this blink...
			g_engine->getUISprite()->Blit(x-11, y-10, 168, 261, 11, 11);
			break;
		case SHIELD_YELLOW_NOSHAREDEXP:
			g_engine->getUISprite()->Blit(x-11, y-10, 168, 261, 11, 11);
			break;
		case SHIELD_BLUE_NOSHAREDEXP_BLINK:
			// TODO (nfries88): actually make this blink...
			g_engine->getUISprite()->Blit(x-11, y-10, 179, 261, 11, 11);
			break;
		case SHIELD_BLUE_NOSHAREDEXP:
			g_engine->getUISprite()->Blit(x-11, y-10, 179, 261, 11, 11);
			break;
		default:
			break;
	}

	uint32_t emblem =  n->getEmblem();
	switch(emblem) {
		case EMBLEM_GREEN:
			g_engine->getUISprite()->Blit(x, y+1, 287, 218, 11, 11);
			break;
		case EMBLEM_RED:
			g_engine->getUISprite()->Blit(x, y+1, 298, 218, 11, 11);
			break;
		case EMBLEM_BLUE:
			g_engine->getUISprite()->Blit(x, y+1, 309, 218, 11, 11);
			break;
		default:
			break;
	}
}

void CreatureUI::getWalkOffset(float &walkoffx, float &walkoffy, float scale) const
{
	Creature* n = (Creature*)this;
	float tilesize = 32*scale;
	switch(n->getLookDir()) {
		case DIRECTION_WEST:
			walkoffx = -m_walkState * tilesize + (m_preWalk ? 0 : tilesize);
			break;
		case DIRECTION_EAST:
			walkoffx = m_walkState * tilesize - (m_preWalk ? 0 : tilesize);
			break;
		case DIRECTION_NORTH:
			walkoffy = -m_walkState * tilesize + (m_preWalk ? 0 : tilesize);
			break;
		case DIRECTION_SOUTH:
			walkoffy = m_walkState * tilesize - (m_preWalk ? 0 : tilesize);
			break;
		case DIRECTION_NE:
			walkoffx = m_walkState * tilesize - (m_preWalk ? 0 : tilesize);
			walkoffy = -m_walkState * tilesize + (m_preWalk ? 0 : tilesize);
			break;
		case DIRECTION_SE:
			walkoffx = m_walkState * tilesize - (m_preWalk ? 0 : tilesize);
			walkoffy = m_walkState * tilesize - (m_preWalk ? 0 : tilesize);
			break;
		case DIRECTION_NW:
			walkoffx = -m_walkState * tilesize + (m_preWalk ? 0 : tilesize);
			walkoffy = -m_walkState * tilesize + (m_preWalk ? 0 : tilesize);
			break;
		case DIRECTION_SW:
			walkoffx = -m_walkState * tilesize + (m_preWalk ? 0 : tilesize);
			walkoffy = m_walkState * tilesize - (m_preWalk ? 0 : tilesize);
			break;
	}
}
void CreatureUI::advanceWalk(int groundspeed)
{
	Creature *n = (Creature*)this;
	if (m_walkState >= 1) {
		m_walkState = 1;
		return;
	}

	m_walkState += (n->getSpeed() / (float)groundspeed) * (g_frameDiff/1000.);
	if (m_walkState >= 1)
		m_walkState = 1;
}

void CreatureUI::loadOutfit()
{
    Creature* n = (Creature*)this;
//    DEBUGPRINT(0,0,"Loading creature %d (itemlook %d)\n", n->getOutfit().m_looktype, n->getOutfit().m_lookitem);

    unloadGfx();

    setupObject();

    if (!m_obj)
        return;

	Outfit_t outfit = n->getOutfit();

	for(uint32_t i = 0; i < m_obj->numsprites ; i++){
		Sprite* spr;
		if(m_obj->blendframes > 1){
			 // if it's a template, then let's just put a NULL in there
            if((i / (m_obj->height * m_obj->width)) % 2 ) {
                m_gfx.insert(m_gfx.end(), NULL);
                continue;
            }
            else{
            	ASSERT(i + m_obj->height * m_obj->width < m_obj->numsprites);
            	ASSERT(m_obj->imageData)

            	spr = g_engine->createSprite("Tibia.spr", m_obj->imageData[i]);
            	Sprite* templatespr = g_engine->createSprite("Tibia.spr", m_obj->imageData[i + m_obj->height * m_obj->width]);
				spr->templatedColorize(templatespr, outfit.m_lookhead, outfit.m_lookbody, outfit.m_looklegs, outfit.m_lookfeet);
				delete templatespr;
            }
        }
        else{
            ASSERT(m_obj->imageData)
        	spr = g_engine->createSprite("Tibia.spr", m_obj->imageData[i]);
        }
		m_gfx.insert(m_gfx.end(), spr);
	}
}

bool CreatureUI::isLoaded() const {
    if (!m_gfx.size()) {
        if (!m_obj) {
            printf("No object, need to reload gfx\n");
            return false;
        }
        if (m_obj->numsprites > 0) {
            printf("Need to reload gfx\n");
            return false;
        }
    }
    return true;
}

void CreatureUI::setupObject() {
    Creature* n = (Creature*)this;
	Outfit_t outfit = n->getOutfit();
	if (!m_obj) {
        if(!outfit.m_looktype && !outfit.m_lookitem){
            m_obj = NULL;
            unloadGfx();
            return;
        }
        else if(outfit.m_looktype == 0 && outfit.m_lookitem != 0){
            //TODO
            m_obj = NULL;
            //m_obj = Objects::getInstance()->getItemType(outfit.m_lookitem);
            return;
        }
        else{
            m_obj = Objects::getInstance()->getOutfitType(outfit.m_looktype);
        }
        if (!m_obj) {
            printf("Could not load outfit of type %d\n", outfit.m_looktype);
            return;
        }
        if (!m_obj->isGfxLoaded()) {
            m_obj->loadGfx();
        }
	}
	if(!m_obj)printf("FAILED to create outfit.\n");
}
