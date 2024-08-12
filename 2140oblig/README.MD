Read from master file table: 
    Vi kaller på en hjelpemetode kalt read_inodes som rekursivt leser inn inodene fra mft, kobler disse sammen, og legger de i minnet. 

Implementation requirements:
    Vi tror at alle implementasjonskravene er møtt.

Precode deviation:
    Vi har laget read_inodes som returnerer en inode (root) og som rekursivt linker inodene sammen og legger til minne.
    Vi har en metode som heter verified_delete_in_parent som tar en parent og en node som parameter, den reallokerer minnet til parent->children, og reduserer parent->num_children med 1.
    
Test: 
    Etter hva vi kan se fungerer alle testene som de skal, med null minne lekkasje.