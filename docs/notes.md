Redstone Implementation
=======================

These are notes about how Redstone appears to be implemented in Minecraft 1.7.  I'm using the code from CraftBukkit at [85f5776df2](https://github.com/Bukkit/CraftBukkit/tree/85f5776df2a9c827565e799f150ae8a197086a98).  Some code has been inlined or refactored for clarity.

Redstone
--------

For a starting point, I'll be looking at [BlockRedstoneWire.java](https://github.com/Bukkit/CraftBukkit/blob/85f5776df2a9c827565e799f150ae8a197086a98/src/main/java/net/minecraft/server/BlockRedstoneWire.java).
Just from looking at the code, there are a large amount of calls to [`World.applyPhysics`](https://github.com/Bukkit/CraftBukkit/blob/85f5776df2a9c827565e799f150ae8a197086a98/src/main/java/net/minecraft/server/World.java#L434) which in turn calls [`World.e`](https://github.com/Bukkit/CraftBukkit/blob/85f5776df2a9c827565e799f150ae8a197086a98/src/main/java/net/minecraft/server/World.java#L469) for each of the surround blocks.
This in turn calls back to [`Block.doPhysics`](https://github.com/Bukkit/CraftBukkit/blob/85f5776df2a9c827565e799f150ae8a197086a98/src/main/java/net/minecraft/server/Block.java#L430) which is overridden in each class.
Looking at [BlockRedstoneWire.doPhysics](https://github.com/Bukkit/CraftBukkit/blob/85f5776df2a9c827565e799f150ae8a197086a98/src/main/java/net/minecraft/server/BlockRedstoneWire.java#L232) we see that it calls [`BlockRedstoneWire.e`](https://github.com/Bukkit/CraftBukkit/blob/85f5776df2a9c827565e799f150ae8a197086a98/src/main/java/net/minecraft/server/BlockRedstoneWire.java#L40) which in turn calls [`BlockRedstoneWire.a](https://github.com/Bukkit/CraftBukkit/blob/85f5776df2a9c827565e799f150ae8a197086a98/src/main/java/net/minecraft/server/BlockRedstoneWire.java#L53) before calling [`World.applyPhysics`](https://github.com/Bukkit/CraftBukkit/blob/85f5776df2a9c827565e799f150ae8a197086a98/src/main/java/net/minecraft/server/BlockRedstoneWire.java#L40) again to propigate.

So it appears that `BlockRedstoneWire.a` performs the actual redstone logic and perpetuates out to other blocks via `World.applyPhysics`.  Let's take a look at how `BlockRedstoneWire.a` works:

```java
private void a(World world, int i, int j, int k, int l, int i1, int j1) {
    // Grab the current 'power' of this block
    int k1 = world.getData(i, j, k);

    // Returns 0 if this block has changed types,
    // otherwise returns it's current 'power'
    byte b0 = 0;
    int l1 = this.getPower(world, l, i1, j1, b0);

    // Temporarily unpowers this block and computes the highest redstone
    // signal nearby.  Max value of 15, minimum value of 0.
    this.a = false;
    int i2 = world.getHighestNeighborSignal(i, j, k);
    this.a = true;

    // If the surrounding redstone signal is greater than our block's signal
    // use the surrounding signal.
    if (i2 > 0 && i2 > l1 - 1) {
        l1 = i2;
    }

    int j2 = 0;
    for (int index = 0; index < 4; ++index) {
        int l2 = i;
        int i3 = k;

        if (index == 0) {
            l2 = i - 1;
        }

        if (index == 1) {
            ++l2;
        }

        if (index == 2) {
            i3 = k - 1;
        }

        if (index == 3) {
            ++i3;
        }

        if (l2 != l || i3 != j1) {
            j2 = this.getPower(world, l2, j, i3, j2);
        }

        if (world.getType(l2, j, i3).r() && !world.getType(i, j + 1, k).r()) {
            if ((l2 != l || i3 != j1) && j >= i1) {
                j2 = this.getPower(world, l2, j + 1, i3, j2);
            }
        } else if (!world.getType(l2, j, i3).r() && (l2 != l || i3 != j1) && j <= i1) {
            j2 = this.getPower(world, l2, j - 1, i3, j2);
        }
    }

    // Redstone strength decays over distance
    if (j2 > l1) {
        l1 = j2 - 1;
    } else if (l1 > 0) {
        --l1;
    } else {
        l1 = 0;
    }

    // Use the surrounding redstone signal if it's greater than the calculated signal.
    if (i2 > l1 - 1) {
        l1 = i2;
    }

    // CraftBukkit start
    if (k1 != l1) {
        BlockRedstoneEvent event = new BlockRedstoneEvent(world.getWorld().getBlockAt(i, j, k), k1, l1);
        world.getServer().getPluginManager().callEvent(event);

        l1 = event.getNewCurrent();
    }
    // CraftBukkit end
    if (k1 != l1) {
        world.setData(i, j, k, l1, 2);
        this.b.add(new ChunkPosition(i, j, k));
        this.b.add(new ChunkPosition(i - 1, j, k));
        this.b.add(new ChunkPosition(i + 1, j, k));
        this.b.add(new ChunkPosition(i, j - 1, k));
        this.b.add(new ChunkPosition(i, j + 1, k));
        this.b.add(new ChunkPosition(i, j, k - 1));
        this.b.add(new ChunkPosition(i, j, k + 1));
    }
}

public int getHighestNeighborSignal(int i, int j, int k) {
    int l = 0;

    for (int i1 = 0; i1 < 6; ++i1) {
        int j1 = this.getBlockFacePower(i + Facing.b[i1], j + Facing.c[i1], k + Facing.d[i1], i1);

        if (j1 >= 15) {
            return 15;
        }

        if (j1 > l) {
            l = j1;
        }
    }

    return l;
}
```

`block.d()` is `true` by default but set to `false` in:

* BlockButtonAbstract
* Cactus
* Cake
* Cocoa
* DaylightDetector
* DiodeAbstract
* Door
* DragonEgg
* EnderPortal
* Fire
* Hopper
* Lever
* Piston
* PistonExtension
* Portal
* PressurePlateAbstract
* RedstoneWire
* Reed
* Sign
* Skull
* Snow
* Soil
* Trapdoor
* Tripwire
* TripwireHook
* Vine

`block.isPowerSource()` is `false` by default but is set to `true` in:

* ButtonAbstract
* DaylightDetector
* DiodeAbstract
* Lever
* MinecartDetector
* PressurePlateAbstract
* RedstoneTorch
* RedstoneWire
* TripwireHook

The one intersting case is in `RedstoneWire` where it's toggled off in the call to `a(..)` during `world.getHighestNeighborSignal(...)`.

```java
// World.java 2435
public int getBlockFacePower(int i, int j, int k, int l) {
    Block block = this.getType(i, j, k);

    // Only tile entries seem to behave differently for determining power testing
    boolean checkPower;
    if (block instanceof TileEntry) {
        checkPower = ((TileEntry)block).r();
    } else {
        checkPower = block.material.k() && block.d() && !block.isPowerSource()
    }

    if (checkPower) {
        return this.getBlockPower(i, j, k)
    } else {
        // Defaults to '0' in Block
        return this.getType(i, j, k).b(this, i, j, k, l);
    }
}
```


