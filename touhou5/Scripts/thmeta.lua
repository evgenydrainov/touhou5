---@meta

---@class texture_handle
---@class music_handle
---@class instance_id
---@class array<T>: {[number]: T}
---@class phase: {HP: number, Time: number, Type: integer, Script: fun()}
---@class boss: {Name: string, Texture: texture_handle, Phases: array<phase>, Healthbars: array<array<integer>>, Music: music_handle}
---@class enemy: {HP: number, Texture: texture_handle, Music: music_handle, Script: fun()}

PHASE_NONSPELL  = 0
PHASE_SPELLCARD = 1

---@param x number
---@param y number
---@param spd number
---@param dir number
---@param acc number
---@param texture texture_handle
---@param radius number
---@param rotate boolean
---@return instance_id
function Shoot(x, y, spd, dir, acc, texture, radius, rotate) end

---@param desc boss
---@return instance_id
function CreateBoss(desc) end

---@param x number
---@param y number
---@param desc enemy
---@return instance_id
function CreateEnemy(x, y, desc) end

---@param fname string
---@return texture_handle
function LoadTexture(fname) end

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
---@return boolean
function BltExists(id) end

---@param id instance_id
---@return boolean
function EnmExists(id) end

---@param id instance_id
---@return boolean
function BossExists(id) end
