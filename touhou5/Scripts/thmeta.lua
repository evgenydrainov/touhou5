---@meta

---@class array<T>: {[integer]: T}
---@class texture_handle
---@class sprite_handle
---@class instance_id
---@class phase: {HP: number, Time: number, Type: integer, Name: string?, Script: fun(id: instance_id): nil}
---@class boss: {Name: string, Sprite: sprite_handle, Phases: array<phase>, Healthbars: array<integer>, Music: string?}
---@class stage: {Music: string?, Script: fun(): nil}

PHASE_NONSPELL  = 0
PHASE_SPELLCARD = 1

PLAY_AREA_W = 384
PLAY_AREA_H = 448

BOSS_STARTING_X = PLAY_AREA_W / 2
BOSS_STARTING_Y = 96

---@param x number
---@param y number
---@param spd number
---@param dir number
---@param acc number
---@param texture texture_handle | -1
---@param radius number
---@param rotate boolean
---@return instance_id
function Shoot(x, y, spd, dir, acc, texture, radius, rotate) end

---@param x number
---@param y number
---@param spd number
---@param dir number
---@param acc number
---@param texture texture_handle | -1
---@param length number
---@param thickness number
---@return instance_id
function ShootLazer(x, y, spd, dir, acc, texture, length, thickness) end

---@param x number
---@param y number
---@param spd number
---@param dir number
---@param acc number
---@param texture texture_handle | -1
---@param radius number
---@return instance_id
function ShootRect(x, y, spd, dir, acc, texture, radius) end

---@param desc boss
---@return instance_id
function CreateBoss(desc) end

---@param desc boss
---@return instance_id
function CreateMidboss(desc) end

---@param x number
---@param y number
---@param radius number
---@param desc boss
---@param midboss boolean
---@return instance_id
function CreateBossEx(x, y, radius, desc, midboss) end

---@param x number
---@param y number
---@param spd number
---@param dir number
---@param acc number
---@param hp number
---@param sprite sprite_handle | -1
---@param script function?
---@return instance_id
function CreateEnemy(x, y, spd, dir, acc, hp, sprite, script) end

---@param x number
---@param y number
---@param spd number
---@param dir number
---@param acc number
---@param radius number
---@param hp number
---@param sprite sprite_handle | -1
---@param script function?
---@return instance_id
function CreateEnemyEx(x, y, spd, dir, acc, radius, hp, sprite, script) end

---@param fname string
---@return texture_handle
function LoadTexture(fname) end

---@param fname string
---@param frame_count integer
---@param anim_spd number
---@param loop_frame integer
---@return sprite_handle
function LoadSprite(fname, frame_count, anim_spd, loop_frame) end

function Random() end

--
-- bullet
--

---@param id instance_id
---@return number
function BltGetX(id) end

---@param id instance_id
---@return number
function BltGetY(id) end

---@param id instance_id
---@return number
function BltGetSpd(id) end

---@param id instance_id
---@return number
function BltGetDir(id) end

---@param id instance_id
---@return number
function BltGetAcc(id) end

---@param id instance_id
---@return number
function BltGetRadius(id) end

---@param id instance_id
---@return number
function BltGetLifetime(id) end

---@param id instance_id
---@return texture_handle
function BltGetTex(id) end



---@param id instance_id
---@param x number
function BltSetX(id, x) end

---@param id instance_id
---@param y number
function BltSetY(id, y) end

---@param id instance_id
---@param spd number
function BltSetSpd(id, spd) end

---@param id instance_id
---@param dir number
function BltSetDir(id, dir) end

---@param id instance_id
---@param acc number
function BltSetAcc(id, acc) end

---@param id instance_id
---@param radius number
function BltSetRadius(id, radius) end

---@param id instance_id
---@param texture texture_handle
function BltSetTex(id, texture) end

--
-- enemy
--

---@param id instance_id
---@return number
function EnmGetX(id) end

---@param id instance_id
---@return number
function EnmGetY(id) end

---@param id instance_id
---@return number
function EnmGetSpd(id) end

---@param id instance_id
---@return number
function EnmGetDir(id) end

---@param id instance_id
---@return number
function EnmGetAcc(id) end

---@param id instance_id
---@return number
function EnmGetRadius(id) end



---@param id instance_id
---@param x number
function EnmSetX(id, x) end

---@param id instance_id
---@param y number
function EnmSetY(id, y) end

---@param id instance_id
---@param spd number
function EnmSetSpd(id, spd) end

---@param id instance_id
---@param dir number
function EnmSetDir(id, dir) end

---@param id instance_id
---@param acc number
function EnmSetAcc(id, acc) end

---@param id instance_id
---@param radius number
function EnmSetRadius(id, radius) end

--
-- player
--

---@return number
function PlrGetX() end

---@return number
function PlrGetY() end

--
-- boss
--

---@param id instance_id
---@return number
function BossGetX(id) end

---@param id instance_id
---@return number
function BossGetY(id) end

---@param id instance_id
---@return number
function BossGetSpd(id) end

---@param id instance_id
---@return number
function BossGetDir(id) end

---@param id instance_id
---@return number
function BossGetAcc(id) end

---@param id instance_id
---@return number
function BossGetRadius(id) end

---@param id instance_id
---@return integer
function BossGetPhase(id) end

---@param id instance_id
---@return sprite_handle
function BossGetSpr(id) end

---@param id instance_id
---@return boolean
function BossIsActive(id) end



---@param id instance_id
---@param x number
function BossSetX(id, x) end

---@param id instance_id
---@param y number
function BossSetY(id, y) end

---@param id instance_id
---@param spd number
function BossSetSpd(id, spd) end

---@param id instance_id
---@param dir number
function BossSetDir(id, dir) end

---@param id instance_id
---@param acc number
function BossSetAcc(id, acc) end

---@param id instance_id
---@param radius number
function BossSetRadius(id, radius) end

---@param id instance_id
---@param sprite sprite_handle
function BossSetSpr(id, sprite) end



---@param id instance_id
---@return boolean
function BltExists(id) end

---@param id instance_id
---@return boolean
function EnmExists(id) end

---@param id instance_id
---@return boolean
function BossExists(id) end



function BGCamSetPos(x, y, z) end
function BGCamSetTarget(x, y, z) end
function BGFogSetOrigin(x, y, z) end
function BGFogSetColor(r, g, b, a) end
function BGFogSetNear(near) end
function BGFogSetFar(far) end
function BGFogEnable(enable) end
