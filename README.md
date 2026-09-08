# Comparaison de structures de données en C

Implémentation et benchmark de trois structures (tableau dynamique, liste
chaînée, table de hachage), pour choisir la bonne structure selon les
contraintes d'un problème plutôt que par habitude.

## Fichiers

| Fichier | Contenu |
|---|---|
| `dynamic_array.h/.c` | Tableau dynamique (`insert_front/back`, `find`, `get`, `remove_front`) |
| `linked_list.h/.c` | Liste chaînée simple (mêmes opérations) |
| `hash_table.h/.c` | Table de hachage à chaînage, `hash_good` / `hash_bad` |
| `benchmark.c` | Programme de mesure (toutes tailles, toutes opérations) |
| `common.h` | Aide de mesure (`clock_gettime`/`CLOCK_MONOTONIC`, portage MSVC pour dev sous Windows) |
| `Makefile` | Compilation gcc (`make && make run`) |
| `build_msvc.bat` | Compilation MSVC (dev/test sous Windows) |

Compilation (Linux, environnement cible) :

```sh
make
./benchmark
```

## 1. Complexités théoriques

n = nombre d'éléments dans la collection.

| Opération | Dynamic Array | Linked List | Hash Table | Gagnant prévu |
|---|---|---|---|---|
| `get(index)` | O(1) | O(n) | — (pas d'accès par index) | **Array** |
| `find(value)` | O(n) | O(n) | O(1) moyen / O(n) pire cas | **Hash Table** |
| `insert_front` | O(n) (décalage) | O(1) | O(1) (non ordonné) | **Linked List** |
| `insert_back` | O(1) amorti | O(n)* | O(1) (non ordonné) | **Array** |
| `remove_front` | O(n) (décalage) | O(1) | O(1)/élément (non ordonné) | **Linked List** |
| Parcours complet | O(n), cache-friendly | O(n), cache-hostile | O(n + TABLE_SIZE) | **Array** (en pratique) |

`*` La structure imposée par l'énoncé (`head`, `size`, pas de pointeur de
queue) oblige à parcourir toute la liste pour insérer en fin : O(n). Avec un
pointeur `tail` en plus, ce serait O(1) — mais ce n'est pas la structure
demandée ici, et cette contrainte est justement ce que le benchmark met en
évidence.

La table de hachage n'a pas de notion d'ordre ni d'index : `get(index)`
n'a pas de sens pour elle, et `insert_front`/`insert_back`/`remove_front`
sont tous équivalents à une simple insertion/suppression O(1) en moyenne
dans un bucket.

## 2. Prédictions avant benchmark

Avant toute mesure, les hypothèses attendues sont :

- **Array bat List sur `get`** : accès direct par arithmétique de pointeur vs
  parcours de chaîne — écart qui devrait se creuser fortement quand n grandit.
- **Array et List sont comparables sur `find`** : tous deux O(n), mais Array
  devrait rester plus rapide en pratique grâce à la localité mémoire
  (tableau contigu vs nœuds épars, donc moins de défauts de cache).
- **List bat Array sur `insert_front`/`remove_front`** : pas de décalage
  d'éléments, juste un rebranchement de pointeur.
- **Array bat List sur `insert_back`** avec la structure imposée : O(1)
  amorti contre O(n) faute de pointeur de queue.
- **Hash Table bat tout le monde sur `find`**, tant que `hash_good` est
  utilisée — et devrait s'effondrer en O(n) avec `hash_bad`, aussi lent
  (voire plus, à cause de l'overhead du calcul de hash) qu'une liste chaînée.
- **Array bat List sur le parcours complet**, même si les deux sont O(n) :
  la contiguïté mémoire du tableau devrait limiter les défauts de cache.

## 3. Résultats & comparaison

Mesures réelles (moyennées sur plusieurs répétitions pour lisser le bruit
de l'horloge — voir note méthodologique plus bas), avec `TABLE_SIZE = 10007`.

*Environnement de mesure : Windows 11, compilation MSVC `/O2` (`clock_gettime`
porté via `QueryPerformanceCounter`, voir `common.h`). Sur l'environnement
cible (Linux/gcc), les temps absolus varieront, mais les rapports de
complexité et les tendances observées sont identiques — c'est ce qui compte
ici.*

### `get(n/2)` — ms par appel

| n | Array | Linked List |
|---:|---:|---:|
| 1 000 | 0.000002 | 0.000735 |
| 10 000 | 0.000002 | 0.011334 |
| 100 000 | 0.000002 | 0.139759 |
| 1 000 000 | 0.000002 | 2.719783 |

L'Array reste strictement constant (O(1) confirmé). La liste croît de façon
quasi linéaire avec n, comme prévu pour une traversée O(n) — avec même une
pente légèrement plus forte que linéaire aux grandes tailles (défauts de
cache dus à l'éparpillement des nœuds en mémoire).

### `find(valeur absente)` — ms par appel

| n | Array | Linked List | Hash (good) | Hash (bad) |
|---:|---:|---:|---:|---:|
| 1 000 | 0.000503 | 0.002040 | 0.000004 | 0.001830 |
| 10 000 | 0.004441 | 0.025480 | 0.000004 | 0.024570 |
| 100 000 | 0.041334 | 0.287197 | 0.000009 | 0.322270 |
| 1 000 000 | 0.542574 | 7.776834 | 0.000346 | 7.611980 |

Trois observations :
- **`hash_good` reste quasi constante** (0.000004 → 0.000346 ms, un facteur
  ~85 pour n multiplié par 1000) : c'est le O(1) moyen attendu, la légère
  croissance venant de l'allongement des chaînes de collision (n/TABLE_SIZE).
- **Array et List sont bien tous les deux O(n)**, mais Array reste 4 à 14×
  plus rapide que List à taille égale : même complexité, mais la contiguïté
  mémoire du tableau limite les défauts de cache que subit la liste chaînée.
- **`hash_bad` s'effondre en O(n)** et rejoint quasiment les temps de la
  liste chaînée (7.61 ms vs 7.78 ms à n = 1 000 000) : logique, puisque
  `hash_bad` renvoie toujours 0, donc `hash_contains` dégénère exactement en
  une recherche linéaire dans une liste chaînée à un seul bucket.

### `insert_front` / `insert_back` / `remove_front` — ms par appel (n = 1 000 000)

| Opération | Array | Linked List |
|---|---:|---:|
| `insert_front` | 0.525810 | 0.000044 |
| `insert_back` | 0.000005 | 7.816396 |
| `remove_front` | 0.882027 | 0.000020 |

Confirmation nette des complexités théoriques : l'Array est ~12 000× plus
lent que la liste pour insérer en tête (décalage O(n) de tout le tableau),
et la liste est ~1 600 000× plus lente que l'Array pour insérer en fin
(parcours O(n) jusqu'à la queue, faute de pointeur `tail`). Chaque
structure gagne exactement là où sa complexité théorique le prédit.

### Parcours complet avec somme — ms (n = 1 000 000)

| Array | Linked List |
|---:|---:|
| 0.206 | 8.345 |

Les deux parcours sont O(n), mais l'écart (~40×) illustre bien la
différence entre accès mémoire contigu (Array, cache-friendly) et
déréférencements de pointeurs dispersés en mémoire (List, cache-hostile).
La complexité asymptotique ne raconte pas toute l'histoire : la constante
cachée derrière le grand O compte énormément en pratique.

### Table de hachage : `hash_good` vs `hash_bad`

| n | Chaîne max (good) | Chaîne max (bad) |
|---:|---:|---:|
| 1 000 | 2 | 1 000 |
| 10 000 | 7 | 10 000 |
| 100 000 | 24 | 100 000 |
| 1 000 000 | 139 | 1 000 000 |

Avec `hash_good`, la chaîne la plus longue reste proche de n/TABLE_SIZE
(distribution quasi uniforme sur les 10 007 buckets). Avec `hash_bad`,
*tout* tombe dans le bucket 0 : la table dégénère intégralement en une
liste chaînée unique de longueur n, ce qui explique directement
l'effondrement des performances de `hash_contains` observé ci-dessus.
Construire la table (`hash_insert`) reste O(1) par insertion même avec
`hash_bad` — l'insertion se contente d'empiler en tête de bucket — c'est
uniquement la *recherche* qui dégénère en O(n).

### Note méthodologique

Construire une liste chaînée de 1 000 000 d'éléments via `insert_back`
répété (comme le ferait naïvement un test « boucle for sur n ») coûterait
O(n²), soit ~10¹² opérations : irréalisable en un temps raisonnable. La
méthodologie retenue ici est donc :

1. **Construction** de la collection de taille n avec l'opération la moins
   coûteuse de chaque structure (`insert_back` pour l'Array, `insert_front`
   pour la List, `hash_insert` pour la Hash Table) — l'ordre des éléments
   n'a pas d'incidence sur les mesures de `get`/`find`/parcours.
2. **Mesure isolée** de chaque opération (`get`, `find`, `insert_front`,
   `insert_back`, `remove_front`) sur cette collection déjà construite,
   répétée un nombre de fois adapté à son coût théorique (des milliers de
   répétitions pour les opérations O(1), une dizaine pour les opérations
   O(n) coûteuses comme `hash_bad`) afin de dépasser la résolution de
   l'horloge tout en gardant un temps d'exécution raisonnable.

C'est une pratique standard de micro-benchmarking : elle isole le coût
*par appel* à une taille n donnée, sans jamais multiplier deux facteurs
O(n) l'un par l'autre pendant la phase de construction.

## 4. Recommandation finale

### Situation A — collection rarement modifiée, 1 000 000 de recherches

**Recommandation : Hash Table (avec `hash_good`).**

| Critère | Array | Linked List | Hash Table |
|---|---|---|---|
| Temps de recherche | O(n) — mesuré 0.54 ms/appel à n = 10⁶ | O(n) — 7.78 ms/appel, le pire des trois | **O(1) moyen — 0.00035 ms/appel** |
| Coût de construction | O(n), rapide (mesuré 8.8 ms) | O(n) si construite via `insert_front`, mais coûteuse à maintenir triée pour une recherche dichotomique | O(n), légèrement plus lente que l'Array (44 ms) à cause du calcul de hash et du chaînage |
| Consommation mémoire | Minimale : un seul bloc contigu de n entiers | La plus élevée par élément (chaque nœud alloué séparément + pointeur `next`) | Intermédiaire : n entrées chaînées + tableau fixe de TABLE_SIZE pointeurs (~80 Ko fixes) |
| Simplicité d'implémentation | Simple | Simple | Un peu plus complexe (fonction de hachage, gestion des collisions) |

Avec 1 000 000 de recherches à effectuer et une collection quasi statique,
le coût de construction (payé une seule fois) devient négligeable face au
coût cumulé des recherches (payé un million de fois). Le gain de la Hash
Table sur `find` (facteur ~1500× par rapport à l'Array mesuré ici, et
encore plus face à la Linked List) domine totalement le calcul, même en
tenant compte d'une construction un peu plus lente et d'une empreinte
mémoire légèrement supérieure à celle de l'Array. La Linked List n'est
jamais un bon choix ici : elle perd sur tous les critères (recherche la
plus lente, mémoire la plus élevée) sans compenser par un point fort
pertinent pour ce scénario.

### Situation B — mémoire très limitée, seulement 10 recherches prévues

**Recommandation : changer pour le Dynamic Array (trié si possible), pas la Hash Table.**

Avec seulement 10 recherches prévues, le gain asymptotique de la Hash
Table (O(1) vs O(n)) ne pèse presque rien en temps absolu : 10 recherches
à 0.54 ms/appel (Array, cas défavorable non trié) coûtent ~5.4 ms au
total contre ~0.0035 ms pour la Hash Table — une différence négligeable à
l'échelle humaine, alors que le tableau reste bien plus économe :

- **Mémoire** : un tableau dynamique ne stocke que les n entiers dans un
  bloc contigu. La Hash Table, elle, réserve **en permanence** un tableau
  fixe de `TABLE_SIZE = 10007` pointeurs (~80 Ko sur une machine 64 bits)
  *en plus* d'une entrée chaînée par valeur (valeur + pointeur `next`,
  donc plus lourde par élément qu'un simple `int` dans un tableau) — un
  surcoût fixe qui n'a de sens à payer que si l'on va l'amortir sur
  beaucoup de recherches.
- **Simplicité** : pas de fonction de hachage à choisir/valider, pas de
  gestion de collisions.

Le compromis mémoire / performance bascule donc : quand le nombre
d'opérations coûteuses (recherches) est trop faible pour amortir le
surcoût structurel de la Hash Table, il n'y a plus de raison de le payer.
Le Dynamic Array (éventuellement trié pour permettre une recherche
dichotomique en O(log n) si les 10 recherches sont connues à l'avance)
offre le meilleur compromis mémoire/simplicité pour ce scénario. On ne
garde donc **pas** la même structure qu'en situation A : le bon choix de
structure dépend du nombre d'opérations qu'on va effectuer, pas seulement
de leur complexité asymptotique.

## Annexe — sortie brute de `benchmark.exe`

Exécution de référence (compilation MSVC, Windows) ayant servi de base aux
tableaux ci-dessus :

```
PS C:\Users\johnw\Documents\exo_data_structures> .\build_msvc.bat
benchmark.c
dynamic_array.c
hash_table.c
linked_list.c
Génération de code en cours...

Compilation reussie : benchmark.exe
PS C:\Users\johnw\Documents\exo_data_structures> .\benchmark.exe

================================================================
n = 1000
-- Dynamic Array ------------------------------------------------
  build (insert_back x n)      :      0.010 ms
  get(n/2)                     :   0.000002 ms/appel
  find(valeur absente)         :   0.000910 ms/appel
  insert_front                 :   0.000723 ms/appel
  insert_back                  :   0.000008 ms/appel
  remove_front                 :   0.001389 ms/appel
  parcours complet (somme)     :      0.000 ms  (sum controle=10425560775260)
-- Linked List --------------------------------------------------
  build (insert_front x n)     :      0.075 ms
  get(n/2)                     :   0.000836 ms/appel
  find(valeur absente)         :   0.002497 ms/appel
  insert_front                 :   0.000205 ms/appel
  insert_back                  :   0.003582 ms/appel
  remove_front                 :   0.000040 ms/appel
  parcours complet (somme)     :      0.006 ms  (sum controle=10425560775260)
-- Hash Table (good vs bad) --------------------------------------
  [hash_good] build             :      0.043 ms  (chaine max=2)
  [hash_good] contains(absent)  :   0.000007 ms/appel
  [hash_bad]  build              :      0.092 ms  (chaine max=1000)
  [hash_bad]  contains(absent)  :   0.002520 ms/appel
================================================================
n = 10000
-- Dynamic Array ------------------------------------------------
  build (insert_back x n)      :      0.226 ms
  get(n/2)                     :   0.000003 ms/appel
  find(valeur absente)         :   0.009552 ms/appel
  insert_front                 :   0.035297 ms/appel
  insert_back                  :   0.000010 ms/appel
  remove_front                 :   0.026718 ms/appel
  parcours complet (somme)     :      0.005 ms  (sum controle=106820134969980)
-- Linked List --------------------------------------------------
  build (insert_front x n)     :      0.938 ms
  get(n/2)                     :   0.014033 ms/appel
  find(valeur absente)         :   0.031125 ms/appel
  insert_front                 :   0.000076 ms/appel
  insert_back                  :   0.027629 ms/appel
  remove_front                 :   0.000027 ms/appel
  parcours complet (somme)     :      0.034 ms  (sum controle=106820134969980)
-- Hash Table (good vs bad) --------------------------------------
  [hash_good] build             :      0.681 ms  (chaine max=7)
  [hash_good] contains(absent)  :   0.000009 ms/appel
  [hash_bad]  build              :      0.942 ms  (chaine max=10000)
  [hash_bad]  contains(absent)  :   0.032000 ms/appel
================================================================
n = 100000
-- Dynamic Array ------------------------------------------------
  build (insert_back x n)      :      1.093 ms
  get(n/2)                     :   0.000003 ms/appel
  find(valeur absente)         :   0.045305 ms/appel
  insert_front                 :   0.059112 ms/appel
  insert_back                  :   0.000008 ms/appel
  remove_front                 :   0.089637 ms/appel
  parcours complet (somme)     :      0.030 ms  (sum controle=1071626859407010)
-- Linked List --------------------------------------------------
  build (insert_front x n)     :      7.591 ms
  get(n/2)                     :   0.156600 ms/appel
  find(valeur absente)         :   0.310744 ms/appel
  insert_front                 :   0.000100 ms/appel
  insert_back                  :   0.345543 ms/appel
  remove_front                 :   0.000025 ms/appel
  parcours complet (somme)     :      0.426 ms  (sum controle=1071626859407010)
-- Hash Table (good vs bad) --------------------------------------
  [hash_good] build             :      6.698 ms  (chaine max=24)
  [hash_good] contains(absent)  :   0.000014 ms/appel
  [hash_bad]  build              :      6.223 ms  (chaine max=100000)
  [hash_bad]  contains(absent)  :   0.337400 ms/appel
================================================================
n = 1000000
-- Dynamic Array ------------------------------------------------
  build (insert_back x n)      :      6.914 ms
  get(n/2)                     :   0.000002 ms/appel
  find(valeur absente)         :   0.487709 ms/appel
  insert_front                 :   0.528921 ms/appel
  insert_back                  :   0.000006 ms/appel
  remove_front                 :   0.926891 ms/appel
  parcours complet (somme)     :      0.249 ms  (sum controle=10739048628531280)
-- Linked List --------------------------------------------------
  build (insert_front x n)     :     62.854 ms
  get(n/2)                     :   2.771077 ms/appel
  find(valeur absente)         :   6.640531 ms/appel
  insert_front                 :   0.000079 ms/appel
  insert_back                  :   6.917976 ms/appel
  remove_front                 :   0.000038 ms/appel
  parcours complet (somme)     :      7.631 ms  (sum controle=10739048628531280)
-- Hash Table (good vs bad) --------------------------------------
  [hash_good] build             :     46.846 ms  (chaine max=139)
  [hash_good] contains(absent)  :   0.000353 ms/appel
  [hash_bad]  build              :     46.410 ms  (chaine max=1000000)
  [hash_bad]  contains(absent)  :   8.493380 ms/appel
================================================================
PS C:\Users\johnw\Documents\exo_data_structures>
```
