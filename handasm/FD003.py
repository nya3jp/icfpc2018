from encoding import trace

hand = trace.Trace()

hand.l_move((1, 0, 0), (0, 0, 1))

hand.fission((1, 1, 0), 1)  # 0(1-3) -> 0(3) 1(2)

hand.fission((0, 1, 1), 0)  # 0(3) -> 0() 3()
hand.s_move((15, 0, 0))     # 1

hand.s_move((0, 1, 0))
hand.fission((1, 0, 1), 0)  # 1(2) -> 1() 2()
hand.s_move((0, 0, 15))     # 3

hand.wait()
hand.wait()
hand.s_move((0, 0, 15))     # 2
hand.wait()                 # 3

'''
0: (1, 1)
1: (17, 1)
2: (18, 17)
3: (1, 17)
'''

hand.gvoid((0, -1, 0), (17, 0, 17))
hand.gvoid((1, -1, 0), (-17, 0, 17))
hand.gvoid((0, -1, 1), (-17, 0, -17))
hand.gvoid((0, -1, 1), (17, 0, -17))

hand.s_move((0, -1, 0))
hand.wait()
hand.s_move((0, 0, -15))
hand.s_move((0, 0, -15))

hand.fusion_p((0, 1, 1))
hand.fusion_p((1, 0, 1))
hand.fusion_s((-1, 0, -1))
hand.fusion_s((0, -1, -1))

hand.wait()
hand.s_move((-15, 0, 0))

hand.fusion_p((1, 1, 0))
hand.fusion_s((-1, -1, 0))

hand.l_move((-1, 0, 0), (0, 0, -1))
hand.halt()

hand.encode('FD003.nbt')

