# Simulacija tkanina
Simulacija tkanina pomoću sistema opruga.

![opruge](https://www.ics.uci.edu/~shz/courses/cs114/docs/proj3/images/fig1.jpg)

*Izvor: [UCI CS114: Cloth Simulation using Mass-Spring System](https://www.ics.uci.edu/~shz/courses/cs114/docs/proj3/index.html)*

## Dependencies
Nakon kloniranja repozitorija, potrebno je učitat git pod-repozitorije. Konkretno, koristi se [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) za lakše upravljanje memorijom na grafičkoj kartici. 
```shell script
git submodule init
git submodule update
```

Za prevođenje je također potrebno instalirat [Vulkan SDK](https://vulkan.lunarg.com/sdk/home). Makefile se treba urediti tako da se varijabla **VULKAN_SDK_PATH** postavi na putanju do trenutne instalacije Vulkan SDK-a. 

## Prevođenje i pokretanje
Za prevođenje se koristi standardni makefile. ```make``` prevodi izvorne datoteke u izvršnu datoteku **SimulacijaTkanine**. ```make shaders``` prevodi shadere iz GLSL oblika u SPIR-V izvršni oblik koji se učitavaju na grafičku karticu. Izvršni program (SimulacijaTkanine) pretpostavlja da se izvršni shaderi nalaze u direktoriju *shaders* relativno na putanju u kojoj se program izvršava.

Ukoliko je Vulkan SDK instaliran globalno, dovoljno je samo pokrenut program SimulacijaTkanine. U suprotnom, potrebno je postavit okolišnu (environment) varijablu **LD_LIBRARY_PATH** na putanju do Vulkan biblioteke, te ako je program preveden s podrškom za debugiranje, varijablu **VK_LAYER_PATH** na putanju do Vulkan validacijskih slojeva (validation layers). Komanda ```make test``` automatski postavlja navedene varijable s obziron na postavljeni *VULKAN_SDK_PATH* u Makefile-u te pokreće program.

## Navigacija
Kroz scenu se pogled mijenja micanjem kursora, a kreće se pomoću tipka W, A, S i D, razmaknicom za dizanje, te X za spuštanje. Tipkom F se uključuje mreža linija tkanine, a tipkom G mreža opruga. Budući da se kod iscrtavanja opruga kod svake sličice u grafičku memoriju učitava velika količina podataka, ne preporuča se uključivanje tog iscrtavnja kod više od 100 točaka tkanine (n > 10).

## Scene i broj točaka tkanine
Program opcionalno prima dvije vrijednosti kod pokretanja: redni broj scene (1-3) te broj točaka (n) uz duž jedne dimenzije tkanine. Ukupni broj točaka tkanine je n<sup>2</sup>. Ako se program pokreće pomoću *make*-a, sintaksa za postavljanje navedenih vrijednosti je sljedeća:
```shell script
make test SCENE=1 POINTS=10
```

### Scena 1
![Scena 1](https://raw.githubusercontent.com/filipbudisa/RG-2019-Lab3/master/res/sc1.png)

### Scena 2
![Scena 2](https://raw.githubusercontent.com/filipbudisa/RG-2019-Lab3/master/res/sc2.png)

### Scena 3
![Scena 3](https://raw.githubusercontent.com/filipbudisa/RG-2019-Lab3/master/res/sc3.png)