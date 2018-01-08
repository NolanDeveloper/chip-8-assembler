        ld      v0, 0
        ld      v1, 1
        ld      v2, 1
        jp      .loop
.loop:
        ld      v0, v1
        ld      v1, v2
        add     v2, v0
        jp      .loop
