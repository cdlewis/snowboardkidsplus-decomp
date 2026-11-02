# Third-party source attribution

PSY-Q SDK C was adapted from [Xeeynamo/psyz](https://github.com/Xeeynamo/psyz),
`decomp/` at commit `f8780dcf04c93fa4f41e2c8a838be1a8ee15b6df`. Its MIT notice is preserved in
[licenses/psyz-decomp.txt](licenses/psyz-decomp.txt).

Imported source bodies:

- Texture loading and environment defaults from `decomp/src/libgpu/ext.c`
- Environment getters, primitive dispatch and packet builders from
  `decomp/src/libgpu/sys.c`

- Sequence/SEP close, stop and play wrappers from `decomp/src/libsnd/ssclose.c`,
  `ssstop.c`, `ssplay.c`; VAB wrappers from `vs_vh.c`, `vs_vtc.c`

- Public callback and interrupt-mask wrappers from `decomp/src/libetc/intr.c`
- Root-counter accessors and setup from `decomp/src/libapi/counter.c`
- Voice-volume getters and setters from `decomp/src/libsnd/ut_vvol.c`
- Program-attribute reads from `decomp/src/libsnd/ut_gpa.c`

- `decomp/src/libsnd/vs_mono.c`, `vm_doff.c`, and start wrappers from `ssstart.c`
- SPU allocator initialization and allocation queries from `decomp/src/libspu/s_m_init.c`,
  `s_m_util.c`; voice-volume reads from `s_gvv.c`; register writes, bus-delay
  setters and shared register/address-shift data from `spu.c`
- `decomp/src/libspu/s_it.c`, `s_stm.c`, `s_snv.c`, `s_srv.c`
- Sine/cosine routines and their shared table from `decomp/src/libgte/geo_00.c`,
  `geo_01.c`, and `sincos.c`; the noise-clock setter from `decomp/src/libspu/s_snc.c`
- The complete decoder module, tables and register pointers from
  `decomp/src/libpress/libpress.c`
- CD reset, accessors, callback setters, wrappers and BCD position conversions from `decomp/src/libcd/sys.c`

Headers contain the subset of types and declarations needed here. Imports are
verified against this game's PSY-Q 4.2 objects rather than assumed identical
to the reference SDK. Shared VM/SPU/CD controller state retains its existing
split-data owner. The decoder owns its data and strings in C.
`_SsVmDamperOn` is reconstructed from the local oracle as the counterpart to
`vm_doff.c`; upstream still has an assembly placeholder for it.

`libetc/vmode.c` is reconstructed from the local oracle. The spurious
`SsSetVoiceMask` label at `SetVideoMode + 8` was a signature collision; it
is part of the video-mode setter, not a separate sound function.

Decoder differences verified against this executable:

- The timeout handler prints only the timeout heading and resets the decoder;
  the 4.0 reference's detailed DMA/FIFO diagnostic prints are absent.
- The two otherwise unused words after the cosine table are `0x210E7350` and
  `0x004246AD`, taken from this executable rather than the 4.0 reference.
- The diagnostic strings and their original alignment/padding are preserved.

The sequence play-mode argument uses signed byte extension in this executable.
Its public wrapper and internal callee declarations preserve that observed
signedness rather than copying the reference private header's unsigned type.

The sequence control-change handlers (`cc_7`, `cc_11`, `cc_64`, `cc_91`,
`cc_100`, `cc_101`, `cc_121`) and `midiprog` are reconstructed from this
executable. Those routines remain assembly placeholders in the reference.

The controller multitap check and actuator-info query (`PadChkMtap` and
`PadInfoAct`) are reconstructed from this executable, including the query's
five-entry switch table. The actuator-info records have a five-byte stride;
their pointer occupies offset 4 in the existing controller-command structure.

The GPU packet mode/offset builders use this executable's 4.2 encoding, which
omits the 4.0 reference's legacy GPU-version branches. Texture-page and CLUT
address encoders are reconstructed from the local assembly. Layouts are checked
against loads/stores and copy sizes in this executable; `DRAWENV` is 0x5C bytes.

`GsGetTimInfo` is reconstructed directly from this game's SDK object, including
its separate palette-present and palette-absent paths and word-based TIM cursor.

The SPU address-register reader and DMA-priority setter are reconstructed from
this executable's assembly. SPU allocator metadata, register and bus-control
pointers, and the address-unit shift are defined in their C owners with their
original initial values. Other SPU controller state retains its split-data owner.
