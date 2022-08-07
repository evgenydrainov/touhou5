---@meta

---@class texture_handle
---@class instance_id
---@class array<T>: {[number]: T}
---@class phase: {HP: number, Time: number, Script: fun()}
---@class boss: {Name: string, Texture: texture_handle, Phases: array<phase>}
---@class enemy: {HP: number, Texture: texture_handle, Script: fun()}

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
function CreateBoss(desc) end

---@param x number
---@param y number
---@param desc enemy
---@return instance_id
function CreateEnemy(x, y, desc) end

---@param fname string
---@return texture_handle
function LoadTexture(fname) end

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

---@return number
function PlrGetX() end

---@return number
function PlrGetY() end

---@return number
function BossGetX() end

---@return number
function BossGetY() end

---@return number
function BossGetSpd() end

---@return number
function BossGetDir() end

---@return number
function BossGetAcc() end

---@return number
function BossGetRadius() end

---@param x number
function BossSetX(x) end

---@param y number
function BossSetY(y) end

---@param spd number
function BossSetSpd(spd) end

---@param dir number
function BossSetDir(dir) end

---@param acc number
function BossSetAcc(acc) end

---@param radius number
function BossSetRadius(radius) end
